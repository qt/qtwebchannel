/*
 * These are some very basic tests for the front end API.
 */
'use strict';
var webchannel;

var assert = require('assert');
var WebSocket = require('faye-websocket');


describe('QSON API', function() {
    //some tests really need some time for communication
    this.timeout(10000);

    before(function(done) {
       
        var socket = new WebSocket.Client('ws://127.0.0.1:1337');
        var transport = {};
        socket.on('open', function(event) {
            console.log("Socket connected");
            transport.postMessage = function(data) {
                socket.send(data);
            }
            
            new require('../../src/webchannel/qwebchannel.js').QWebChannel(transport, function(channel) {
                console.log("Webchannel ready");
                webchannel = channel;
                done();
            });
        });
        
        socket.on('message', function(event) {
            transport.onmessage(event);
        });
        
        socket.on('error', function (error) {
            
        });

        socket.on('close', function () {
            
        });
        
        
    });

    beforeEach(function(done) {
        // prepare client
        done();
    });

    afterEach(function() {
        // cleanup client
    });

    
    it('send "NodeJS Test" as a message to the server', function(done) {
        webchannel.objects.dialog.receiveText("NodeJS Test");
        done();
    });
});
