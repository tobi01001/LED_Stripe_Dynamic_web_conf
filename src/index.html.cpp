#include <pgmspace.h>
char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width' />

  <title>WS2812FX Ctrl</title>

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
    color:#454545;
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

  ul#brightness li, ul#speed li, ul#auto ul#colorrgb li {
    display:inline-block;
    width:20%;
  }

  ul li a {
    display:block;
    margin:3px;
    padding:10px 5px;
    border:2px solid #454545;
    border-radius:5px;
    color:#454545;
    font-weight:bold;
    text-decoration:none;
  }

  ul li a.active {
    border:2px solid #909090;
    color:#909090;
  }
  </style>
</head>
<body>
  <h1>WS2812FX Control</h1>
  <canvas id='colorbar' width='75' height='1080'></canvas>
  <div id='controls'>
    <ul id='mode'></ul>

    <ul id='brightness'>
      <li>Brightness</li>
      <li><a href='#' class='br' id='u'>u</a></li>
      <li><a href='#' class='br' id='d'>d</a></li>
    </ul>
    <ul id='speed'>
      <li>Speed</li>
      <li><a href='#' class='sp' id='u'>u</a></li>
      <li><a href='#' class='sp' id='d'>d</a></li>
    </ul>
    <ul id='auto'>
      <li>mode</li>
      <li><a href='#' class='mo' id='u'>u</a></li>
      <li><a href='#' class='mo' id='d'>d</a></li>
    </ul>
    <ul id='colorrgb'>
      <li>Red</li>
      <li><a href='#' class='re' id='u'>u</a></li>
      <li><a href='#' class='re' id='d'>d</a></li>
    </ul>
    <ul id='colorrgb'>
      <li>Green</li>
      <li><a href='#' class='gr' id='u'>u</a></li>
      <li><a href='#' class='gr' id='d'>d</a></li>
    </ul>
    <ul id='colorrgb'>
      <li>Blue</li>
      <li><a href='#' class='bl' id='u'>u</a></li>
      <li><a href='#' class='bl' id='d'>d</a></li>
    </ul>
  </div>
</body>
</html>
)=====";
