import QtQuick 1.0
import Qt.labs.WebChannel 1.0
import QtWebKit 1.0

Rectangle {
    width: 1000
    height: 360
    WebChannel {
        id: webChannel

        onRequest: {
            var data = JSON.parse(requestData   );
            txt.text = data.a;
            response.send(JSON.stringify({b:'This is a response from QML'}));
        }
    }

    WebView {
        id: webView
        anchors.top: txt.bottom
        height: 200
        settings.localContentCanAccessRemoteUrls: true
        settings.developerExtrasEnabled: true
        url: "index.html?webchannel_baseUrl=" + webChannel.baseUrl
    }

    TextEdit {
        width: 1000
        height: 100
        id: editor
        anchors.top: parent.top
    }
    Text {
        id: txt
        anchors.top: editor.bottom
        text: "BLA"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                webChannel.broadcast("incoming-call", JSON.stringify(editor.text));
            }
        }
    }
}
