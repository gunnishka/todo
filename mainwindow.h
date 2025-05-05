#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidgetItem>

// Объявление класса интерфейса (генерируется Qt из .ui файла)
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для кнопок (должны совпадать с именами в Qt Designer)
    void on_addTaskButton_clicked();      // Добавление задачи
    void on_moveTaskButton_clicked();     // Перемещение задачи
    void on_deleteTaskButton_clicked();   // Удаление задачи
    void deleteTask(int taskId);

private:
    Ui::MainWindow *ui;       // Указатель на интерфейс
    QSqlDatabase db;          // Объект базы данных

    // Вспомогательные методы
    void initializeDatabase();    // Инициализация БД
    void refreshTaskList();       // Обновление списка задач
    void loadCategories();        // Загрузка категорий
    int getCurrentTaskId();       // Получение ID выбранной задачи
    void executeQuery(const QString &query); // Выполнение SQL-запросов
    void setupTaskList();          // Настройка виджета
    void setupTaskItem(QListWidgetItem *item, int taskId,
                       const QString &title, const QString &category);
};
#endif // MAINWINDOW_H
