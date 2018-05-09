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
      color:#a0a0a0;
      margin:0;
      padding:0;
    }

    body {
      width:100%;
      max-width:1024px;
      background-color:#202020;
    }

    h3 {
      width:65%;
      margin:25px 0 25px 25%;
      color:#a0a0a0;
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
      display:block;
      width:25%;
    }
	
	 ul#pal li {
      display:block;
      width:25%;
    }

    ul#brightness li, ul#speed li, ul#auto li, ul#colorrgb li {
      display:block;
      width:30%;
    }

    div a {
      display:inline-block;
      margin:3px;
      padding:10px 5px;
      border:2px solid #808080;
      border-radius:5px;
      color:#a0a0a0;
      font-weight:bold;
      text-decoration:none;
    }

    div p a.active {
      border:2px solid #a0a0ff;
      color:#a0a0ff;
    }
    </style>
  </head>
<body>
  <h3>LED_Steuerung</h3>
  <canvas id='colorbar' width='50' height='360'></canvas>
  <div id='controls'>
      <div id='off'>
        <p style='color:#a0a0a0;'>
          <a href='#' class='mo' id='o' style='color:#a0a0a0;'>Off</a>
        </p>
      </div>
      <div id='div_mode'>
    	<p id='mode' style='color:#a0a0a0;'>Effekte


		</p>
      </div>
	  <div id='div_pal'>
    	<p id='pal' style='color:#a0a0a0;'>Paletten


		</p>
      </div>
	  <div id='brightness'>
        <p style='color:#a0a0a0;'>Helligkeit
          <a href='#' class='b' id='d' style='color:#a0a0a0;'>dunkler</a>
          <a href='#' class='b' id='u' style='color:#c0c0c0;'>heller</a>
        </p>
      </div>
      <div id='speed'>
        <p style='color:#a0a0a0;'>Tempo
        	<a href='#' class='s' id='d' style='color:#6060a0;'>langsamer</a>
        	<a href='#' class='s' id='u' style='color:#a06060;'>schneller</a>
        </p>
      </div>
      <div id='auto'>
        <p style='color:#a0a0a0;'>Blend Type
          <a href='#' class='tbl' id='N' style='color:#6060a0;'>No Blend</a>
          <a href='#' class='tbl' id='L' style='color:#a06060;'>Linear Blend</a>
        </p>
        <p style='color:#a0a0a0;'>Auto Mode
          <a href='#' class='am' id='y' style='color:#6060a0;'>Auto</a>
          <a href='#' class='am' id='n' style='color:#a06060;'>Manual</a>
        </p>
        <p style='color:#a0a0a0;'>Auto Palette
          <a href='#' class='ap' id='y' style='color:#6060a0;'>Auto</a>
          <a href='#' class='ap' id='n' style='color:#a06060;'>Manual</a>
        </p>
      </div>
    </div>
  </body>
  </html>
)=====";

