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
    <h3>LED Steuerung</h3>
    <canvas id='colorbar' width='75' height='1080'></canvas>
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
      <div id='brightness'>
        <p style='color:#a0a0a0;'>Helligkeit
          <a href='#' class='br' id='d' style='color:#a0a0a0;'>dunkler</a>
          <a href='#' class='br' id='u' style='color:#c0c0c0;'>heller</a>
        </p>
      </div>
      <div id='speed'>
        <p style='color:#a0a0a0;'>Tempo
        	<a href='#' class='sp' id='u' style='color:#6060a0;'>langsamer</a>
        	<a href='#' class='sp' id='d' style='color:#a06060;'>schneller</a>
        </p>
      </div>
      <div id='auto'>
        <p style='color:#a0a0a0;'>Effekt
          <a href='#' class='mo' id='d' style='color:#6060a0;'>vorheriger</a>
          <a href='#' class='mo' id='u' style='color:#a06060;'>naechster</a>
        </p>
      </div>
      <div id='colorrgb'>
        <p style='color:#c02020;'>Rot
        	<a href='#' class='re' id='u' style='color:#c02020;'>mehr</a></li>
        	<a href='#' class='re' id='d' style='color:#a02020;'>weniger</a>
        </p>
      </div>
      <div id='colorrgb'>
        <p style='color:#20c020;'>Grün
        	<a href='#' class='gr' id='u' style='color:#20c020;'>mehr</a>
        	<a href='#' class='gr' id='d' style='color:#20a020;'>weniger</a>
      </div>
      <div id='colorrgb'>
        <p style='color:#2020c0;'>Blau
        	<a href='#' class='bl' id='u' style='color:#2020c0;'>mehr</a>
        	<a href='#' class='bl' id='d' style='color:#2020a0;'>weniger</a>
        </p>
      </div>
    </div>
  </body>
  </html>
)=====";
