/*
 * PolTest ESP32 Control
 * HTML pages
 */

// > web pages in LittleFS stored as +.html
// only OTA updater remains in string, fallback

const char update_html_fallback[] PROGMEM = R"=====(
<!DOCTYPE html><html lang="en">
<head>
  <meta charset="UTF-8">
  <title>System & File Update</title>
  <link rel="stylesheet" type="text/css" href="/style.css">
</head>
<body style="background-color:#000000; color:#FFFFFF;">
  <div class="topnav">
    <div class="nav"><a href="/index.html">Live</a></div>
    <div class="nav"><a href="/update">Update</a></div>
  </div>
  <header><div class="title">File & System Upload</div></header>
  <main>
    <form method='POST' action='#' enctype='multipart/form-data' id='upload-form' class="grid1">
      <div class="ltext">Select File (*.html, *.css, *.js, *.bin)</div>
      <input type='file' name='upload'>
      <div class="prgback"><div id='prg' class="prgbar">&nbsp;</div></div>
      <input type='submit' value='Upload'>
    </form>
  </main>
<script>
  var prg = document.getElementById('prg');
  var form = document.getElementById('upload-form');
  form.addEventListener('submit', e => {
    e.preventDefault();
    var data = new FormData(form);
    var req = new XMLHttpRequest();
    req.open('POST', '/upload');
    req.upload.addEventListener('progress', p => {
      let w = Math.round((p.loaded / p.total) * 100) + '%';
      if (p.lengthComputable) {
        prg.innerHTML = w;
        prg.style.width = w;
      }
      if (w == '100%') prg.style.backgroundColor = '#000080';
    });
    req.onload = function() {
      if (req.status == 200) alert("Upload sucessful!");
      else alert("Upload failed!");
    };
    req.send(data);
  });
</script>
</body></html>
)=====";
