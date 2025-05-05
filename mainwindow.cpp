#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "taskdialog.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlError>

#include <QMessageBox>
#include <QInputDialog>
#include <QListWidget>
#include <QComboBox>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setFixedSize(1600,900);
    ui->setupUi(this);
    setWindowTitle("Task manager");
    setupTaskList();

    initializeDatabase();
    loadCategories();
    refreshTaskList();
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
}

void MainWindow::setupTaskList()
{
    ui->taskListWidget->setDragEnabled(true);
    ui->taskListWidget->setAcceptDrops(true);
    ui->taskListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    ui->taskListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void MainWindow::initializeDatabase()
{
    // Настройка подключения к SQLite
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tasks.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
        return;
    }
}

void MainWindow::on_addTaskButton_clicked()
{
    TaskDialog dialog(this);
    dialog.setCategories(ui->categoryComboBox->model());

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.prepare(
            "INSERT INTO tasks (title, description, category_id) "
            "VALUES (:title, :desc, (SELECT id FROM categories WHERE name = :category))"
            );
        query.bindValue(":title", dialog.getTitle());
        query.bindValue(":category", dialog.getSelectedCategory());

        if (!query.exec()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить задачу: " + query.lastError().text());
        }
        refreshTaskList();
    }
}

void MainWindow::on_moveTaskButton_clicked()
{
    int taskId = getCurrentTaskId();
    if (taskId == -1) return;

    QString newCategory = ui->categoryComboBox->currentText();

    QSqlQuery query;
    query.prepare(
        "UPDATE tasks SET category_id = "
        "(SELECT id FROM categories WHERE name = :category) "
        "WHERE id = :taskId"
        );
    query.bindValue(":category", newCategory);
    query.bindValue(":taskId", taskId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось переместить задачу!");
    }
    refreshTaskList();
}

void MainWindow::on_deleteTaskButton_clicked()
{
    int taskId = getCurrentTaskId();
    if (taskId == -1) return;

    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE id = :taskId");
    query.bindValue(":taskId", taskId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить задачу!");
    }
    refreshTaskList();
}

void MainWindow::refreshTaskList()
{
    ui->taskListWidget->clear();

    QSqlQuery query(
        "SELECT t.id, t.title, c.name FROM tasks t "
        "JOIN categories c ON t.category_id = c.id"
        );

    while (query.next()) {
        int taskId = query.value(0).toInt();  // Получаем ID задачи
        QString taskTitle = query.value(1).toString();  // Название задачи
        QString categoryName = query.value(2).toString();  // Название категории

        // Создаем контейнерный виджет
        QWidget *customWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(customWidget);
        layout->setContentsMargins(5, 5, 5, 5);  // Восстанавливаем отступы

        // Название задачи
        QLabel *titleLabel = new QLabel(taskTitle);
        titleLabel->setAlignment(Qt::AlignLeft);

        // Категория задачи
        QLabel *categoryLabel = new QLabel(categoryName);
        categoryLabel->setStyleSheet("color: blue; font-weight: bold;");
        categoryLabel->setAlignment(Qt::AlignRight);

        // Кнопка удаления
        QPushButton *deleteButton = new QPushButton("×");
        deleteButton->setStyleSheet("color: red; border: none;");
        deleteButton->setFixedSize(20, 20);

        // Добавляем элементы в макет
        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(categoryLabel);
        layout->addWidget(deleteButton);

        // Создаем элемент списка
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(customWidget->sizeHint());  // Автоматический размер
        ui->taskListWidget->addItem(item);
        ui->taskListWidget->setItemWidget(item, customWidget);

        // Подключаем кнопку удаления
        connect(deleteButton, &QPushButton::clicked, this, [this, taskId]() {
            deleteTask(taskId);  // Вызываем метод удаления
        });
    }
}

void MainWindow::deleteTask(int taskId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(taskId);

    if (query.exec()) {
        refreshTaskList();  // Обновляем список после удаления
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить задачу");
    }
}

void MainWindow::loadCategories()
{
    ui->categoryComboBox->clear();
    QSqlQuery query("SELECT name FROM categories");
    while (query.next()) {
        ui->categoryComboBox->addItem(query.value(0).toString());
    }
}

int MainWindow::getCurrentTaskId()
{
    QListWidgetItem *item = ui->taskListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите задачу!");
        return -1;
    }
    return item->data(Qt::UserRole).toInt();
}

void MainWindow::executeQuery(const QString &query)
{
    QSqlQuery q;
    if (!q.exec(query)) {
        QMessageBox::critical(this, "Ошибка базы данных", q.lastError().text());
    }
}
