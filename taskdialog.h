#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = nullptr);
    void setCategories(QAbstractItemModel *model); // Установка списка категорий

    // Геттеры для получения данных
    QString getTitle() const;
    QString getDescription() const;
    QString getSelectedCategory() const;

private:
    QLineEdit *titleEdit;
    QTextEdit *descEdit;
    QComboBox *categoryCombo;
};

#endif // TASKDIALOG_H
