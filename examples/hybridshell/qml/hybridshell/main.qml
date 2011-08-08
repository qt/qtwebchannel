import QtQuick 1.0
import Qt.labs 1.0
import Qt.labs.WebChannel 1.0
import QtWebKit 1.0

Rectangle {
    HybridShell {
        id: shell
        onStdoutData: {
            console.log(data);
            webChannel.broadcast("stdout", data);
        }
        onStderrData: {
            webChannel.broadcast("stderr", data);
        }
    }

    WebChannel {
        id: webChannel
        onExecute: {
            shell.exec(requestData);
        }

        onBaseUrlChanged: shell.start()
    }

    width: 480
    height: 800

    WebView {
        anchors.fill: parent
        url: "index.html?webChannelBaseUrl=" + webChannel.baseUrl
        onAlert: console.log(message)
    }
}
