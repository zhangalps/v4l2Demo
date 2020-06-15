import QtQuick 2.0
import com.syberos.basewidgets 2.0
import QtQuick.Controls 1.4

 Rectangle {
    property bool isShow: false
    property alias text: t.text     // 显示的文字
    width: 200
    height: 100
    z: 100
    color: "#666666"
    opacity: isShow ? 1 : 0
    //border.width: units.dp(1)
    //border.color: "white"
    radius: width/2

    Behavior on opacity {
        NumberAnimation { duration: 1000 }
    }

//    ColumnLayout {
//        Label {
//            //Layout.margins: t.height
//            Layout.margins: units.dp(15)
//            id: t;
//            color: "white"
//            text: ""
//        }
//    }
    Text{
        id: t
        color: "white"
        text: ""
        anchors.margins:50
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Timer {
        id: toastTimer
        interval: 1000
        onTriggered: isShow = false
    }

    // 显示toast函数
    function showToast(text) {
        t.text = text;
        isShow = true;
        toastTimer.restart();
    }
}

