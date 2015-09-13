#include <QDebug>
#include <QStandardPaths>

#include "costs_nb_qml_proxy.h"

const char *QStr2str(const QString qStr)
{
    return qStr.toUtf8().constData();
}

QStringList stdToQStrList(std::list<std::string> stdStrList)
{
    QStringList qStrList;
    for (auto i = stdStrList.cbegin(); i != stdStrList.end(); ++i) {
        qStrList.append(QString::fromStdString(*i));
    }
    return qStrList;
}

#define REAL_DB_NAME "costs_nb_db.json"
#define TEST_DB_NAME "test_db.json"
#define COSTS_NB_DB_NAME REAL_DB_NAME
#define COSTS_NB_CFG_NAME "costs_nb.cfg"

Costs_nb_qml_proxy::Costs_nb_qml_proxy(QQmlContext *qmlContext)
    : _qmlContext(qmlContext)
{
    QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    costs_nb_core = new CostsNbCore(QStr2str(dbLocation + "/" + COSTS_NB_DB_NAME), QStr2str(dbLocation + "/" + COSTS_NB_CFG_NAME));
    update_qml_categoriesModel();
    update_qml_showExpensesModel("");
}

Costs_nb_qml_proxy::~Costs_nb_qml_proxy()
{
    delete costs_nb_core;
}

std::tuple<QStringList, QString> Costs_nb_qml_proxy::current_categories()
{
    std::list<std::string> currentCategoriesCore;
    std::string currentCategoriesParent;
    std::tie(currentCategoriesCore, currentCategoriesParent) = costs_nb_core->GetCurrentCategories();
    QStringList currentCategoriesUI = stdToQStrList(currentCategoriesCore);
    return std::make_tuple(currentCategoriesUI, QString::fromStdString(currentCategoriesParent));
}

bool Costs_nb_qml_proxy::category_selected(const QString &selectedCategory)
{
    bool res = costs_nb_core->CategorySelected(QStr2str(selectedCategory));
    update_qml_categoriesModel();
    return res;
}

void Costs_nb_qml_proxy::category_back()
{
    costs_nb_core->CategoryBack();
    update_qml_categoriesModel();
}

bool Costs_nb_qml_proxy::category_remove(const QString &removingCategory)
{
    bool res = costs_nb_core->RemoveCategory(QStr2str(removingCategory));
    update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_add(const QString &newCategory)
{
    bool res = costs_nb_core->CategoryAdd(QStr2str(newCategory));
    if (res)
        update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_add_sub(const QString &parentCategory, const QString &newCategory)
{
    bool res = costs_nb_core->CategoryAddSub(QStr2str(parentCategory), QStr2str(newCategory));
    if (res)
        update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_rename(const QString &selectedCategory, const QString &newName)
{
    bool res = costs_nb_core->RenameCategory(QStr2str(selectedCategory), QStr2str(newName));
    if (res)
        update_qml_categoriesModel();
    return res;
}

void Costs_nb_qml_proxy::update_qml_categoriesModel()
{
    QStringList currentCategoriesCore;
    QString currentCategoriesParent;
    std::tie (currentCategoriesCore, currentCategoriesParent) = current_categories();
    _qmlContext->setContextProperty("categoriesModel", QVariant::fromValue(currentCategoriesCore));
    _qmlContext->setContextProperty("categoriesHeader", QVariant::fromValue(currentCategoriesParent));
}

void Costs_nb_qml_proxy::update_qml_showExpensesModel(const QString &selectedCategory)
{
    _qmlContext->setContextProperty("showExpensesModel", QVariant::fromValue(stdToQStrList(costs_nb_core->GetExpenses(QStr2str(selectedCategory)))));
}

void Costs_nb_qml_proxy::update_qml_showAllExpensesModel(const QString &selectedCategory)
{
    _qmlContext->setContextProperty("showExpensesModel", QVariant::fromValue(stdToQStrList(costs_nb_core->GetAllExpenses(QStr2str(selectedCategory)))));
}

void Costs_nb_qml_proxy::buy(const QString &selectedCategory, const unsigned int &cost, const QString &info)
{
    costs_nb_core->Buy(QStr2str(selectedCategory), cost, QStr2str(info));
    update_qml_categoriesModel();
}
