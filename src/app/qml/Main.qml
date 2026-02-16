import QtQuick
import QtQuick.Window

Window {
    visible: true
    width: 400
    height: 600
    color: "black"

    Image {
        id: imageView
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: "qrc:/qml/lenna.bmp"
    }
}
