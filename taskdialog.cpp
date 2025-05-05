#include "taskdialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDialogButtonBox>

#include <QDebug>

TaskDialog::TaskDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Новая задача");
    setFixedSize(400,300);
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Поле ввода названия
    titleEdit = new QLineEdit(this);
    titleEdit->setPlaceholderText("Название задачи");
    layout->addWidget(new QLabel("Название:"));
    layout->addWidget(titleEdit);

    // Выбор категории
    categoryCombo = new QComboBox(this);
    layout->addWidget(new QLabel("Категория:"));
    layout->addWidget(categoryCombo);

    // Кнопки OK/Cancel
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
        );
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void TaskDialog::setCategories(QAbstractItemModel *model)
{
    categoryCombo->setModel(model);
}

QString TaskDialog::getTitle() const { return titleEdit->text(); }
QString TaskDialog::getDescription() const { return descEdit->toPlainText(); }
QString TaskDialog::getSelectedCategory() const { return categoryCombo->currentText(); }
