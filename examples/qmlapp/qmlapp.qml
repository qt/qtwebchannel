import QtQuick 1.0
import Qt.labs.WebChannel 1.0
import QtWebKit 1.0

Rectangle {
    width: 1000
    height: 360
    WebChannel {
        id: webChannel

        onExecute: {
            var data = JSON.parse(requestData);
            txt.text = data.a;
            response.send(JSON.stringify({b:'This is a response from QML'}));
        }

        onBaseUrlChanged: {
            console.log(baseUrl);
        }
    }

    WebView {
        id: webView
        url: "index.html?webChannelBaseUrl=" + webChannel.baseUrl   ;
        settings.developerExtrasEnabled: true
        anchors.top: txt.bottom
        height: 200
        width: 200
    }

    TextEdit {
        width: 1000
        height: 100
        id: editor
        anchors.top: parent.top
    }
    Text {
        id: txt
        text: "Click"
        anchors.top: editor.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: {
                webChannel.broadcast("incoming-call", JSON.stringify(editor.text));
            }
        }
    }
}
