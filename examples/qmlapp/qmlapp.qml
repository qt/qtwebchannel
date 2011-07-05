import QtQuick 1.0
import Qt.labs.WebChannel 1.0
import QtWebKit 1.0

Rectangle {
    width: 1000
    height: 360
    WebChannel {
        id: webChannel
        useSecret: false
        onRequest: {
            var data = JSON.parse(request);
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
        url: "index.html?baseUrl=" + webChannel.baseUrl
    }

    Text {
        id: txt
        anchors.top: parent.top
    }
}
