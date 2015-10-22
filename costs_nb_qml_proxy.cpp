#include <QDebug>
#include <QStandardPaths>

#include "costs_nb_qml_proxy.h"

QStringList stdToQStrList(std::list<std::string> stdStrList)
{
    QStringList qStrList;
    for (auto i = stdStrList.cbegin(); i != stdStrList.end(); ++i) {
        qStrList.append(QString::fromStdString(*i));
    }
    return qStrList;
}

#define COSTS_NB_CFG_NAME "costs_nb.cfg"

Costs_nb_qml_proxy::Costs_nb_qml_proxy(QQmlContext *qmlContext)
    : _qmlContext(qmlContext)
{
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString dbLocation = writableLocation;
    QString cfgFullName = dbLocation + "/" + COSTS_NB_CFG_NAME;
    ini_cfg = new IniConfig(cfgFullName.toStdString());
    ini_cfg->SetValue("log", "log_dir", writableLocation.toStdString());

    QString dbName = QString::fromStdString(ini_cfg->GetValue("database", "db_name"));
    QString dbFullName = dbLocation + "/" + dbName;
    CategoriesToJsonFileConverter * categories_to_db_converter =
      new CategoriesToJsonFileConverter(dbFullName.toStdString(),
        new CategoriesToJsonConverterJsoncppLib());

    CategoriesToBackend * categories_to_backend = new CategoriesToRestfulBackend(
          new CategoriesToJsonConverterJsoncppLib(), ini_cfg);

    costs_nb_core = new CostsNbCore(categories_to_db_converter, categories_to_backend);
    update_qml_categoriesModel();
    update_qml_showExpensesModel("");
}

Costs_nb_qml_proxy::~Costs_nb_qml_proxy()
{
    delete costs_nb_core;
    delete ini_cfg;
}

std::tuple<QStringList, QString> Costs_nb_qml_proxy::current_categories()
{
    std::list<std::string> categoriesStr;
    std::string categoriesParentStr;
    std::tie(categoriesStr, categoriesParentStr) = costs_nb_core->GetCurrentCategories();
    QStringList currentCategoriesUI = stdToQStrList(categoriesStr);
    return std::make_tuple(currentCategoriesUI, QString::fromStdString(categoriesParentStr));
}

bool Costs_nb_qml_proxy::category_selected(const QString &selectedCategory)
{
    bool res = costs_nb_core->CategorySelected(selectedCategory.toStdString());
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
    bool res = costs_nb_core->RemoveCategory(removingCategory.toStdString());
    update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_add(const QString &newCategory)
{
    bool res = costs_nb_core->CategoryAdd(newCategory.toStdString());
    if (res)
        update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_add_sub(const QString &parentCategory, const QString &newCategory)
{
    bool res = costs_nb_core->CategoryAddSub(parentCategory.toStdString(), newCategory.toStdString());
    if (res)
        update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_rename(const QString &selectedCategory, const QString &newName)
{
    bool res = costs_nb_core->RenameCategory(selectedCategory.toStdString(), newName.toStdString());
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
    auto expensesStr = costs_nb_core->GetExpenses(selectedCategory.toStdString());
    _qmlContext->setContextProperty("showExpensesModel", QVariant::fromValue(stdToQStrList(expensesStr)));
}

void Costs_nb_qml_proxy::update_qml_showAllExpensesModel(const QString &selectedCategory)
{
    auto expensesStr = costs_nb_core->GetAllExpenses(selectedCategory.toStdString());
    _qmlContext->setContextProperty("showExpensesModel", QVariant::fromValue(stdToQStrList(expensesStr)));
}

void Costs_nb_qml_proxy::buy(const QString &selectedCategory, const unsigned int &cost, const QString &info)
{
    costs_nb_core->Buy(selectedCategory.toStdString(), cost, info.toStdString());
    update_qml_categoriesModel();
}

bool Costs_nb_qml_proxy::category_move_back(const QString &selectedCategory)
{
    bool res = costs_nb_core->MoveCategoryBack(selectedCategory.toStdString());
    update_qml_categoriesModel();
    return res;
}

bool Costs_nb_qml_proxy::category_move_to(const QString &selectedCategory, const QString &toCategory)
{
    bool res = costs_nb_core->MoveCategoryTo(selectedCategory.toStdString(), toCategory.toStdString());
    update_qml_categoriesModel();
    return res;
}

void Costs_nb_qml_proxy::update_qml_moveCategoryToModel()
{
    QStringList currentCategoriesCore;
    std::tie (currentCategoriesCore, std::ignore) = current_categories();
    _qmlContext->setContextProperty("moveCategoryToModel", QVariant::fromValue(currentCategoriesCore));
}
