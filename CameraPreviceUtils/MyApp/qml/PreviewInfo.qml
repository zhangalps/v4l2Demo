import QtQuick 2.0
import QtDocGallery 5.0
import com.syberos.basewidgets 2.0
import com.syberos.multimedia.photos 2.0


//MessageBrowseMain {
//    id: messageBrowseMain
//    rightButtonText: os.i18n.ctr(qsTr("发送")) // "发送"

//}
PhotoPreview {
    id: __cameraPreviewMain

    signal hideStatusBar
    signal pageShown
    signal pageHidden
    property string folderPath : "/home/user/DCIM/Camera"
    signal progressmChanged(double value)
    titleBarRightItemEnabled: true
    viewmodel: DocumentGalleryModel {
        id: gallery

        autoUpdate: true
        rootType: DocumentGallery.Visual
        filter:  GalleryFilterIntersection {
            GalleryContainsFilter {
                 property: "url"
                 value: __cameraPreviewMain.folderPath
            }
            GalleryFilterUnion {
                    GalleryWildcardFilter {
                        property: "mimeType";
                        value: "image/*"
                    }
                    GalleryEqualsFilter{
                        property: "mimeType";
                        value: "video/mp4"
                    }
             }
        }
        sortProperties: "-lastModified"

        properties: [ "url", "fileName", "mimeType", "fileSize", "lastModified", "width", "height"]

        onProgressChanged: {
            console.log("[camera] [doc gallery] progress changed: ", progress);
            progressmChanged(progress);
        }
        onStatusChanged: {
            console.log("[camera] [doc gallery] status changed: ", status);
        }

        property int lastCount: 0
        onCountChanged: {
            console.log("[camera] [doc gallery] ====== count changed from: ", lastCount , " to: ", count);

            if (lastCount < count) {
                console.log("[camera] [doc gallery] ======  update current index: ", __cameraPreviewMain.initIndex ,", set initIndex to 0")
                __cameraPreviewMain.initIndex = 0;
            }
            lastCount = count;
            console.log("[camera] [doc gallery] ======  initIndex: ", __cameraPreviewMain.initIndex, " lastCount: ", lastCount)
        }
    }

    onReachBegin: {
        pageStack.pop();
    }

    Component.onDestruction: {
        __cameraPreviewMain.hideStatusBar()
        console.log("[camera][pthosviewpage]  destruction")
    }

    onStatusChanged: {
        console.log("[camera] preview page status changed to: ", status)

        if (status == CPageStatus.WillShow) {
            console.log("[camera] preview page will hide...")
        } else if (status == CPageStatus.WillHide) {
            console.log("[camera] preview page will hide...")
        } else if (status == CPageStatus.Show) {
            console.log("[camera] preview page shown...", __cameraPreviewMain.initIndex)
            pageShown()
        } else if (status == CPageStatus.Hide) {
            console.log("[camera] preview page hidden ")
            pageHidden()
        }
    }
}

