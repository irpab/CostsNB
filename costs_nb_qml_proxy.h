#ifndef COSTS_NB_QML_PROXY_H
#define COSTS_NB_QML_PROXY_H

#include <QObject>
#include <QQmlContext>
#include <QStringList>

#include "ini_config.h"
#include "costs_nb_core.h"
#include "categories_to_json_converter.h"
#include "categories_to_backend.h"

class Costs_nb_qml_proxy : public QObject
{
    Q_OBJECT

//    Q_PROPERTY(QStringList current_categories READ current_categories)

    void update_qml_categoriesModel();
    std::tuple<QStringList, QString> current_categories();

    AbstractConfig* ini_cfg;
    CostsNbCore *costs_nb_core;
    QQmlContext *_qmlContext;
public:
    explicit Costs_nb_qml_proxy(QQmlContext *qmlContext = 0);
    ~Costs_nb_qml_proxy();

    Q_INVOKABLE bool category_selected(const QString &selectedCategory);
    Q_INVOKABLE void category_back();
    Q_INVOKABLE bool category_add(const QString &newCategory);
    Q_INVOKABLE bool category_remove(const QString &removingCategory);
    Q_INVOKABLE bool category_add_sub(const QString &parentCategory, const QString &newCategory);
    Q_INVOKABLE bool category_rename(const QString &selectedCategory, const QString &newName);
    Q_INVOKABLE void buy(const QString &selectedCategory, const unsigned int &cost, const QString &info);
    Q_INVOKABLE void update_qml_showExpensesModel(const QString &selectedCategory);
    Q_INVOKABLE void update_qml_showAllExpensesModel(const QString &selectedCategory);

signals:

public slots:
};

#endif // COSTS_NB_QML_PROXY_H
