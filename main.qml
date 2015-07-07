import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: appWindow
    title: qsTr("CostsNB")
    width: 480
    height: 640
    visible: true

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: spacing
            Button {
                text: "Back"
                onClicked: costs_nb_qml_proxy.category_back();
            }
            Button {
                text: "Add"
                onClicked: {
                    new_category_dialog_type = "add";
                    newCategoryDialog.open();
                }
            }
            Button {
                text: "Buy"
                onClicked: {
                    tapped_category = "";
                    buyDialog.open();
                }
            }
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        ListView {
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            id: view

            header: Rectangle {
                width: view.width
                height: 120
                border {
                    color: "blue"
                    width: 1
                }

                Text {
                    anchors.centerIn: parent
                    renderType: Text.NativeRendering
                    font.pixelSize: 110
                    text: categoriesHeader
                    color: "blue"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        messageDialog.show("Choose Category")
                    }
                }
            }

            model: categoriesModel
            delegate: Item {
                id: fruitDelegate
                width: view.width
                height: 120
                Rectangle {
                    anchors.margins: 5
                    anchors.fill: parent
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 100
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            tapped_category = modelData; // avoid this
                            var category_select_res = costs_nb_qml_proxy.category_selected(modelData);
                            if (category_select_res) {
                                buyDialog.open();
                            }
                        }
                        onPressAndHold: {
                            //messageDialog.show(modelData);
                            tapped_category = modelData; // avoid this
                            categoryTapHoldMenu.popup()
                        }
                    }
                }
            }
        }
    }

    MessageDialog {
        id: messageDialog
        title: qsTr("Info")

        function show(caption) {
            messageDialog.text = caption;
            messageDialog.open();
        }
    }

    property var new_category_dialog_type: "";
    property var tapped_category: "";
    Dialog {
        id: newCategoryDialog
        title: "Add New Category"
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        TextField {
            id: inputCategory
            maximumLength: 20
            placeholderText: qsTr("Enter Category")
        }

        onButtonClicked: {
            Qt.inputMethod.hide();
            if (clickedButton == StandardButton.Ok) {
                var category_add_res = false;
                if (new_category_dialog_type == "add")
                    category_add_res = costs_nb_qml_proxy.category_add(inputCategory.text);
                else if (new_category_dialog_type == "add_sub")
                    category_add_res = costs_nb_qml_proxy.category_add_sub(tapped_category, inputCategory.text);
                inputCategory.placeholderText = qsTr("Enter Category");
                tapped_category = "";
                if (!category_add_res) {
                    messageDialog.show("Adding Failed")
                }
            }
            inputCategory.text = "";
        }
    }

    Dialog {
        id: newRenameCategoryDialog
        title: "Rename Category"
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        TextField {
            id: renameCategory
            maximumLength: 20
        }

        onButtonClicked: {
            Qt.inputMethod.hide();
            if (clickedButton == StandardButton.Ok) {
                var category_rename_res = costs_nb_qml_proxy.category_rename(tapped_category, renameCategory.text);
                tapped_category = "";
                if (!category_rename_res) {
                    messageDialog.show("Renaming Failed")
                }
            }
            renameCategory.text = "";
        }
    }

    Menu {
        id: categoryTapHoldMenu
        title: qsTr("&Category Manipulating")
        MenuItem {
            text: qsTr("&Add Sub-Category")
            onTriggered: {
                new_category_dialog_type = "add_sub";
                newCategoryDialog.open();
            }
        }
        MenuItem {
            text: qsTr("&Remove Category")
            onTriggered: {
                var category_rm_res = costs_nb_qml_proxy.category_remove(tapped_category);
                if (!category_rm_res) {
                    messageDialog.show("Removing Failed")
                }
            }
        }
        MenuItem {
            text: qsTr("&Rename Category")
            onTriggered: {
                if (tapped_category.substring(0, 2) == "> ") {
                    renameCategory.text = tapped_category.substring(2);
                } else {
                    renameCategory.text = tapped_category;
                }
                newRenameCategoryDialog.open();
            }
        }
        MenuItem {
            text: qsTr("&Show Expenses")
            onTriggered: {
                costs_nb_qml_proxy.update_qml_showExpensesModel(tapped_category);
                showExpensesDialog.open();
            }
        }
        MenuItem {
            text: qsTr("&Show All Expenses")
            onTriggered: {
                costs_nb_qml_proxy.update_qml_showAllExpensesModel(tapped_category);
                showExpensesDialog.open();
            }
        }
    }

    Dialog {
        id: buyDialog
        title: "Buy"
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        SpinBox {
            id: buyCost
            Layout.fillWidth: true
            menu: null
            maximumValue: 1000000
            value: 0
        }

        onButtonClicked: {
            Qt.inputMethod.hide();
            if (clickedButton == StandardButton.Ok) {
                costs_nb_qml_proxy.buy(tapped_category, Math.round(buyCost.value));
            }
            buyCost.value = 0;
        }
    }

    Dialog {
        id: showExpensesDialog
        title: "Expenses"
        standardButtons: StandardButton.Ok

        Rectangle {
            implicitWidth: appWindow.width*0.8
            implicitHeight: appWindow.height*0.8
            anchors.centerIn: parent
            ListView {
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                id: showExpensesListview

                model: showExpensesModel
                delegate: Item {
                    id: showExpensesDelegate
                    width: showExpensesListview.width
                    height: 50
                    Rectangle {
                        anchors.margins: 3
                        anchors.fill: parent
                        Text {
                            text: modelData
                            font.pixelSize: 45
                        }
                    }
                }
            }
        }
    }
}
