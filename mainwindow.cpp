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
        query.bindValue(":desc", dialog.getDescription());
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

void MainWindow::on_addCategoryButton_clicked()
{
    bool ok;
    QString categoryName = QInputDialog::getText(
        this,
        "Новая категория",
        "Введите название категории:",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (ok && !categoryName.isEmpty()) {
        QSqlQuery query;
        query.prepare("INSERT INTO categories (name) VALUES (:name)");
        query.bindValue(":name", categoryName);

        if (!query.exec()) {
            QMessageBox::warning(this, "Ошибка", "Такая категория уже существует!");
        } else {
            loadCategories();  // Обновляем список категорий
        }
    }
}

void MainWindow::refreshTaskList() {
    ui->taskListWidget->clear(); // Очистка списка задач

    QSqlQuery query("SELECT tasks.title, categories.name FROM tasks "
                    "JOIN categories ON tasks.category_id = categories.id");

    while (query.next()) {
        QString taskTitle = query.value(0).toString();
        QString categoryName = query.value(1).toString();

        // Создаем пользовательский виджет
        QWidget *customWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(customWidget);
        layout->setContentsMargins(0, 0, 0, 0); // Убираем отступы

        // Название задачи
        QLabel *titleLabel = new QLabel(taskTitle);
        titleLabel->setAlignment(Qt::AlignLeft); // Выравнивание задачи по левому краю

        // Категория задачи
        QLabel *categoryLabel = new QLabel(categoryName);
        categoryLabel->setStyleSheet("color: blue; font-weight: bold;"); // Установить цвет и стиль категории
        categoryLabel->setAlignment(Qt::AlignRight); // Выравнивание категории по правому краю

        // Добавляем виджеты в макет
        layout->addWidget(titleLabel);
        layout->addStretch(); // Распределяет пространство между элементами
        layout->addWidget(categoryLabel);

        customWidget->setLayout(layout);

        // Создаем элемент списка и добавляем кастомный виджет
        QListWidgetItem *item = new QListWidgetItem(ui->taskListWidget);
        QSize itemSize(400, 50); // Указываем ширину (400) и высоту (50)
        item->setSizeHint(itemSize);
        ui->taskListWidget->setItemWidget(item, customWidget);
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

void MainWindow::on_deleteCategoryButton_clicked()
{
    // Получаем текущую категорию
    QString categoryToDelete = ui->categoryComboBox->currentText();
    if (categoryToDelete.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите категорию для удаления!");
        return;
    }

    // Удаляем категорию из базы данных (если необходимо)
    QSqlQuery query;
    query.prepare("DELETE FROM categories WHERE name = :category");
    query.bindValue(":category", categoryToDelete);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить категорию из базы данных!");
        return;
    }

    // Удаляем категорию из ComboBox
    int indexToDelete = ui->categoryComboBox->currentIndex();
    if (indexToDelete != -1) {
        ui->categoryComboBox->removeItem(indexToDelete);
    }

    QMessageBox::information(this, "Успех", "Категория успешно удалена!");

    // Обновляем список задач
    refreshTaskList();
}

