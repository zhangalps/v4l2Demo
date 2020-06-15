import QtQuick 2.0

import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import com.syberos.basewidgets 2.0

CPage {
    id: cameraPage
    width: parent.width
    height:parent.height
    color:"black"

    contentAreaItem: Rectangle {

        Column{
            anchors.left: parent.left
            anchors.topMargin: 200
            width: 200
            height: 200
            spacing: 30

            CButton{
                id: f1Btn
                width: 200
                height: MSingletonTool.btnHeight
                textColor: MSingletonTool.txtColor
                text: "F1"
                pixelSize: 42
            }

            CButton{
                id: f2Btn
                width: 200
                height: MSingletonTool.btnHeight
                textColor: MSingletonTool.txtColor
                text: "F2"
                pixelSize: 42
            }
        }

    } //contentAreaItem: Item end

}
