﻿$(document).ready(function () {
var socket = new WebSocket("ws://127.0.0.1:7777");
var test = 2;

$("#test_butt").click(function () {
  argin = ['a','b','c'];
  
  argin = new Array();
  argin.push("sadsadsad");
  argin.push("bbbcjvhgdf");
  argin.push("1112");
  argin.push(25);
  
  argin = {};
  //argin.argin = [23,556,8886,333];
  argin.command = "DevShort";
   argin.argin = 23;
  //socket.send("HELLLLOOOOOOOO");
  var sender = JSON.stringify(argin);
  socket.send(sender);
  console.log("but clicked");
});


socket.onmessage = function(event) {
  //alert("Получены данные " + event.data);
  $("#test").html("Получены данные " + event.data + "<br><br>");
  //console.log("Получены данные " + event.data);
};

socket.onerror = function(error) {
  $("#test").append("Ошибка " + error.message);
  socket.close();
};

socket.onclose = function(event) {
  if (event.wasClean) {
    $("#test").append('Соединение закрыто чисто');
    socket.close();
  } else {
    $("#test").append('Обрыв соединения'); // например, "убит" процесс сервера
    socket.close();
  }
  $("#test").append('Код: ' + event.code + ' причина: ' + event.reason);
  socket.close();
};
});