import QtQuick 2.0
import com.syberos.basewidgets 2.0
import QtQuick.Controls 1.4


CPageStackWindow {

    id: appWindow
    width: 1920
    height:1080
    keyHandle:false
    property var previewBg;

    //     initialPage: MainPage { }
    initialPage:CPage {
        width:1920
        height:1080
        Connections{
            target: appWindow
            onBackKey:{
                console.log("jake zhg onBackKey"+switchDialog.showAnimating)
                if(switchDialog.showAnimating){
                    switchDialog.hide();
                }
                CodeImage.backKey();
                appQuit();
                Qt.quit();

            }
        }

        contentAreaItem:Rectangle{
            Rectangle{
                id:imageRect
                width: 1280
                height:960

                //                rotation:openCamera.switchOn & switchCamera.text === "后置相机"  ? 180 : 0
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    id:img
                    //                    mirror: openCamera.switchOn & switchCamera.text === "前置相机"
                    width: parent.width
                    height:parent.height
                    source: "qrc:/res/MyApp.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Item{
                anchors.left: parent.left
                height:100
                Text{
                    id:openText
                    text:"打开相机"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize :gUiConst.getValue("S3") * gUiConst.getValue("density")
                }
                CSwitch {
                    id: openCamera
                    visible: true
                    anchors.left:openText.right
                    onSwitchOnChanged:{
                        console.log(" zhg onSwitchOnChanged------:"+ switchOn);
                        if(switchOn){
                            CodeImage.openCamera();
                        }else{
                            CodeImage.stopCamera();
                        }

                    }
                }
            }
            CButton {
                anchors.right : parent.right
                id:switchCamera
                text: CodeImage.getCameraPosition() === 0 ?"后置相机" :"前置相机"
                onClicked:{
                    console.log("=========switchCamera showAnimating::" + switchDialog.visible)
                    CodeImage.switchCamera(text)
                    toast.showToast("切换摄像头");
                    if(!switchDialog.showAnimating){
                        switchDialog.visible = true;
                        switchDialog.show();
                        timerA.start();
                    }
                    console.log("=========switchCamera.showAnimating " + switchDialog.showAnimating)


                }
            }

            Timer {
                id: timerA
                repeat: false
                interval: 3000
                onTriggered: {
                    console.log("=========switchDialog.showAnimating " + switchDialog.showAnimating +
                                " hideAnimating: "+ switchDialog.hideAnimating + " switchDialog.visible: "+switchDialog.visible)

                    if(switchDialog.visible){
                        switchDialog.hide();
                        switchDialog.visible = false;
                    }
                    console.log("=========switchDialog222222222.showAnimating " + switchDialog.visible)
                }
            }
            Connections{
                target: CodeImage
                onCallQmlRefeshImg:{
                    //                    console.log("jake zhg onCallQmlRefeshImg ~~~~")
                    img.source = ""
                    img.source = "image://myImage/"+Math.random()
                    if(switchDialog.visible){
                        console.log("onCallQmlRefeshImg showAnimating: true ::" + switchDialog.hideAnimating);
                        switchDialog.hide();
                        switchDialog.visible = false
                    }
                    console.log("onCallQmlRefeshImg zhg************ rotation: "+imageRect.rotation + " img.mirror: "+img.mirror
                                +" switchCamera:" + switchCamera.text + " bool:"+!(imageRect.rotation === 180));
                    if(openCamera.switchOn & switchCamera.text === "前置相机" & !img.mirror){
                        img.mirror = true;
                        imageRect.rotation = 0;
                    }else if(openCamera.switchOn & switchCamera.text === "后置相机" & !(imageRect.rotation === 180)){
                        img.mirror = false;
                        imageRect.rotation = 180;
                    }

                }
                onCallQmlCameraStatus:{
                    console.log("jake zhg oncallQmlCameraStatus" + status)
                    open.text = status;
                }
                onCallQmlCameraPosition:{
                    console.log("jake zhg onCallQmlCameraPosition" + position)
                    toast.showToast(position);
                    if(position === "后置相机" || position ==="前置相机" ){
                        switchCamera.text = position;
                    }else{
                        switchDialog.visible = false;
                    }

                }
                onCallQmlCaptureSuc:{
                    console.log("jake zhg onCallQmlCaptureSuc")
                    __buttonIcon.source = previewBg
                }
            }

            CIndicatorDialog
            {
                id: switchDialog
                anchors.fill: parent
                messageText: "切换中..."
                Component.onCompleted: hide();
            }



            Row{
                anchors.bottom: parent.bottom
                anchors.margins: 30
                anchors.horizontalCenter: parent.horizontalCenter
                width: camera.width * 4
                height: 80
                spacing: 30

                CButton {
                    id: __previewButton

                    width: 80
                    height: 80
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 35
                    Rectangle {
                        id: __buttonContainer
                        width: parent.width
                        height: 80
                        radius: 80 / 2
                        clip:true

                        Image {
                            id: __buttonIcon
                            width: parent.width
                            height: parent.height
                            source: "qrc:/res/chakan.png"
                            sourceSize: Qt.size(env.dp(256), env.dp(341))
                            smooth: true
                        }
                    }
                    onClicked: {
                        console.log(" zhg click preview ")
                        openCamera.switchOn=false
                        pageStack.push("qrc:/qml/PreviewInfo.qml")
                    }
                }

                CButton{
                    id: camera
                    width: 200
                    height: parent.height
                    textColor: "red"
                    text: "拍照"
                    pixelSize: 42
                    onClicked:{
                        console.log("jake zhg camera ~~~~")
                        CodeImage.startCameraCapture();
                        toast.showToast("开始拍照");
                        previewBg = img.source

                    }
                }

                CButton{
                    id: video
                    width:200
                    height: parent.height
                    textColor: "green"
                    text: "录像"
                    pixelSize: 42
                    onClicked:{
                        console.log("jake zhg video ~~~~")
                        CodeImage.startRecorder();
                        toast.showToast("开始录像");


                    }
                }

                CButton{
                    id: stop
                    width: 200
                    height: parent.height
                    textColor: "blue"
                    text: "停止录像"
                    pixelSize: 42
                    onClicked:{
                        console.log("jake zhg stop camera ~~~~")
                        CodeImage.stopRecorder();
                        toast.showToast("停止录像");


                    }
                }

            }
        }
    }
    Toast{
        id:toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: toast.height
    }
//    onBackKey: {
//        console.log("jake onBackKey needs quite");
//        appQuit();
//    }
}




