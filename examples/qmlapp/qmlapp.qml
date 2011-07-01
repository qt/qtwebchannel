import QtQuick 1.0
import Qt.labs.WebChannel 1.0
import QtWebKit 2.0

Rectangle {
    width: 1000
    height: 360
    WebChannel {
        id: webus
        useSecret: false
        onRequest: {
            var data = JSON.parse(request);
            response.send(JSON.stringify({b:'goodbye ' + data.a}));
        }
    }

    WebView {
        id: webView
        anchors.top: parent.top
        settings.localContentCanAccessRemoteUrls: true
        settings.developerExtrasEnabled: true
        url: "index.html?baseUrl=" + webus.baseUrl
    }

    Text {
        id: txt
        anchors.top: webView.bottom
    }
}
