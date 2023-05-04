const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
.card{
     max-width: 500px;
     min-height: 200px;
     background: #02b875;
     padding: 30px;
     box-sizing: border-box;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}

</style>
<body>


<div class="card">
  <h1>FWD Power</h1>
  <h1><span id="FWDValue">0</span> dBm</h1>
</div>

<div class="card">
  <h1>REF Power</h1>
  <h1><span id="REFValue">0</span> dBm</h1>
</div>

<div class="card">
  <h1>SWR</h1>
  <h1><span id="SWRValue">0</span></h1>
</div>

<div class="card">
  <h1>Temperature</h1> 
  <h1><span id="TEMPValue">0</span>&#8451;</h1>
</div>

<form method='get' action='config'><button class='config' value='config' name='config' type='submit'>Configuration</button></form>

<script>

setInterval(function() {
  // Call a function repetatively with 1 Second interval
  getFWD();
  getREF();
  getTEMP();
}, 1000); //1000mSeconds update rate

function getFWD() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("FWDValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readFWD", true);
  xhttp.send();
}
function getREF() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("REFValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readREF", true);
  xhttp.send();
}
function getSWR() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("SWRValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readSWR", true);
  xhttp.send();
}
function getTEMP() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TEMPValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readTEMP", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";