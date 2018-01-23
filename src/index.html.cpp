#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width' />

  <title>LED Lampensteuerung</title>

  <script type='text/javascript' src='main.js'></script>

  <style>
  * {
    font-family:sans-serif;
    margin:0;
    padding:0;
  }

  body {
    width:100%;
    max-width:1024px;
    background-color:#202020;
  }

  h1 {
    width:65%;
    margin:25px 0 25px 25%;
    color:#0a0aa0;
    text-align:center;
  }

  #colorbar {
    float:left;
  }

  #controls {
    width:65%;
    display:inline-block;
    padding-left:5px;
  }

  ul {
    text-align:center;
  }

  ul#mode li {
    display:inline-block;
    width:20%;
  }

  ul#brightness li, ul#speed li, ul#auto li, ul#colorrgb li {
    display:inline-block;
    width:20%;
  }

  ul li a {
    display:block;
    margin:3px;
    padding:10px 5px;
    border:2px solid #0a0aa0;
    border-radius:5px;
    color:#0a0aa0;
    font-weight:bold;
    text-decoration:none;
  }

  ul li a.active {
    border:2px solid #a0a0ff;
    color:#a0a0ff;
  }
  </style>
</head>
<body>
  <h1>WS2812FX Control</h1>
  <canvas id='colorbar' width='75' height='1080'></canvas>
  <div id='controls'>
    <ul id='mode'></ul>

    <ul id='brightness'>
      <licolor=#0a0aa0>Helligkeit</li>
      <li><a href='#' class='br' id='u'>brighter</a></li>
      <li><a href='#' class='br' id='d'>darker</a></li>
    </ul>
    <ul id='speed'>
      <licolor=#0a0aa0>Geschwindigkeit</li>
      <li><a href='#' class='sp' id='u'>slower</a></li>
      <li><a href='#' class='sp' id='d'>faster</a></li>
    </ul>
    <ul id='auto'>
      <licolor=#0a0aa0>Effekt</li>
      <li><a href='#' class='mo' id='u'>next</a></li>
      <li><a href='#' class='mo' id='d'>prev</a></li>
    </ul>
    <ul id='colorrgb'>
      <li color=#a00000>Rot</li>
      <li><a href='#' class='re' id='u'>plus</a></li>
      <li><a href='#' class='re' id='d'>minus</a></li>
    </ul>
    <ul id='colorrgb'>
      <li color=#00a000>Grün</li>
      <li><a href='#' class='gr' id='u'>plus</a></li>
      <li><a href='#' class='gr' id='d'>minus</a></li>
    </ul>
    <ul id='colorrgb'>
      <li color=#0000a0>Blau</li>
      <li><a href='#' class='bl' id='u'>plus</a></li>
      <li><a href='#' class='bl' id='d'>minus</a></li>
    </ul>
  </div>
</body>
</html>
)=====";
