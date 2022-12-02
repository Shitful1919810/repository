const char *responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
const char *responseHeadersPlain = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
const char *html = R"+*(
<html>
  <head>
    <meta charset="utf-8" />
    <title>温度传感器控制页面</title>
    <script defer="defer">
      function ledSwitch() {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            document.getElementById("response").innerHTML =
              xmlhttp.responseText;
          }
        };
        xmlhttp.open("GET", "switch", true);
        xmlhttp.send();
      }
      function setInterval() {
        var xmlhttp = new XMLHttpRequest();
        var valInt = document.getElementById("rangeInterval").value;
        xmlhttp.onreadystatechange = function () {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            document.getElementById("response").innerHTML =
              xmlhttp.responseText;
          }
        };
        var sendInt = valInt;
        if (valInt <= 60000) sendInt = valInt;
        else if (valInt <= 120000) sendInt = (valInt - 60000) * 60;
        else sendInt = (valInt - 120000) * 60 * 60;
        xmlhttp.open("GET", "itv" + sendInt, true);
        xmlhttp.send();
      }
      function setRedLEDInterval() {
        var xmlhttp = new XMLHttpRequest();
        var valInt = document.getElementById("redLEDInterval").value;
        xmlhttp.onreadystatechange = function () {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            document.getElementById("response").innerHTML =
              xmlhttp.responseText;
          }
        };
        xmlhttp.open("GET", "led" + valInt, true);
        xmlhttp.send();
      }
      function changeInterval() {
        var valInt = document.getElementById("rangeInterval").value;
        if (valInt <= 60000)
          document.getElementById("intervalDisplay").innerHTML =
            valInt / 1000.0 + "s";
        else if (valInt <= 120000)
          document.getElementById("intervalDisplay").innerHTML =
            (valInt - 60000) / 1000.0 + "min";
        else
          document.getElementById("intervalDisplay").innerHTML =
            (valInt - 120000) / 1000.0 + "h";
      }
      function changeLEDInterval() {
        var valInt = document.getElementById("redLEDInterval").value;
        document.getElementById("redLEDDisplay").innerHTML =
          valInt / 1000.0 + "s";
      }
      function setNData() {
        var xmlhttp = new XMLHttpRequest();
        var valNData = document.getElementById("rangeNData").value;
        xmlhttp.onreadystatechange = function () {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            document.getElementById("response").innerHTML =
              xmlhttp.responseText;
          }
        };
        xmlhttp.open("GET", "nda" + valNData, true);
        xmlhttp.send();
      }
      function changeNData() {
        var valNData = document.getElementById("rangeNData").value;
        document.getElementById("nDataDisplay").innerHTML = valNData;
      }
      function downloadData() {
        window.open("/download");
      }
      function clearData() {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
          if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            document.getElementById("response").innerHTML =
              xmlhttp.responseText;
          }
        };
        xmlhttp.open("GET", "clr", true);
        xmlhttp.send();
      }
    </script>
  </head>
  <body>
    <div id="txtState">温度检测装置</div>
    <input type="button" value="开关显示器" onclick="ledSwitch()" /><br />
    <input
      type="button"
      value="红色LED指示灯闪烁间隔"
      onclick="setRedLEDInterval()"
    />
    <input
      type="range"
      name="range"
      id="redLEDInterval"
      value="3000"
      min="500"
      max="10000"
      step="500"
      onchange="changeLEDInterval()"
    />
    <span id="redLEDDisplay">3s</span><br />
    <input type="button" value="设置采集间隔" onclick="setInterval()" />
    <input
      type="range"
      name="range"
      id="rangeInterval"
      value="5000"
      min="1000"
      max="144000"
      step="1000"
      onchange="changeInterval()"
    />
    <span id="intervalDisplay">5s</span><br />
    <input type="button" value="设置缓存数据数量" onclick="setNData()" />
    <input
      type="range"
      name="range"
      id="rangeNData"
      value="100"
      min="5"
      max="1000"
      step="1"
      onchange="changeNData()"
    />
    <span id="nDataDisplay">100</span><br />
    <input type="button" value="下载温度数据" onclick="downloadData()" /><br />
    <input type="button" value="清空所有数据" onclick="clearData()" /><br />
    <span id="response"></span>
  </body>
</html>
)+*";
