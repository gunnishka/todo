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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Настройка стилей
    this->setStyleSheet(
        "QMainWindow { background: #f5f5f5; }"
        "QListWidget { background: white; border-radius: 5px; }"
        "QPushButton { background: #4CAF50; color: white; padding: 8px; }"
        );

    initializeDatabase();
    loadCategories();
    refreshTaskList();
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
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

    // Создание таблицы категорий
    executeQuery(
        "CREATE TABLE IF NOT EXISTS categories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE NOT NULL);"
        );

    // Создание таблицы задач
    executeQuery(
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT,"
        "category_id INTEGER,"
        "FOREIGN KEY(category_id) REFERENCES categories(id));"
        );

    // Добавление стандартных категорий
    executeQuery(
        "INSERT OR IGNORE INTO categories (name) VALUES"
        "('Сегодня'), ('Завтра'), ('В ближайшее время');"
        );
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

void MainWindow::refreshTaskList()
{
    ui->taskListWidget->clear();
    QSqlQuery query(
        "SELECT t.id, t.title, c.name FROM tasks t "
        "JOIN categories c ON t.category_id = c.id"
        );

    while (query.next()) {
        QString taskText = QString("%1 [%2]").arg(
            query.value(1).toString(),
            query.value(2).toString()
            );
        QListWidgetItem *item = new QListWidgetItem(taskText, ui->taskListWidget);
        item->setData(Qt::UserRole, query.value(0)); // Сохраняем ID задачи
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
