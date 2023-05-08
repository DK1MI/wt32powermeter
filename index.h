const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
.row {
  width: 100%;
  margin: 0 auto;
}
.box{
     max-width: 500px;
     min-width: 250px;
     min-height: 300px;
     background: #009879;
     padding: 30px;
     box-sizing: border-box;
     vertical-align: top;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
     display: inline-block;
}
.redbox{
  background: #FFAA79;
}

.whitebox{
  background: #FFFFFF;
  color: #009879;
  border-style: solid;
  border-color: #009879;
}

.button{background-color: #009879; border: none; color: white; padding: 5px 5px; text-align: center; text-decoration: none; display: inline-block; margin: 4px 2px; cursor: pointer; border-radius: 8px;}

</style>
<body>

<div class="row">

<div class="box whitebox">
  <h1>Band</h1>
  <h1><span id="BANDValue">0</span></h1>
</div>

<div id="fwd_box" class="box">
  <h1>FWD Power</h1>
  <h1><span id="FWDWatt">0</span> W </br><span id="FWDdBm">0</span> dBm </br><span id="FWDVoltage">0</span> mV</h1>
</div>

<div class="box">
  <h1>REF Power</h1>
  <h1><span id="REFWatt">0</span> W </br><span id="REFdBm">0</span> dBm </br><span id="REFVoltage">0</span> mV</h1>
</div>

<div class="box">
  <h1>SWR</h1>
  <h1>1:<span id="SWRValue">1.0</span></h1>
</div>

<form method='post' action='config'><button class='button' value='config' name='config' type='submit'>Configuration</button></form>

<script>

setInterval(function() {
  // Call a function repetatively
  getDATA();
}, 500); //1000mSeconds update rate

function getDATA() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      const data = this.responseText.split(";");
      document.getElementById("FWDWatt").innerHTML = data[0];
      document.getElementById("FWDdBm").innerHTML = data[1];
      document.getElementById("FWDVoltage").innerHTML = data[2];
      document.getElementById("REFWatt").innerHTML = data[3];
      document.getElementById("REFdBm").innerHTML = data[4];
      document.getElementById("REFVoltage").innerHTML = data[5];
      document.getElementById("SWRValue").innerHTML = data[6];
      //document.getElementById("TEMPValue").innerHTML = data[7];
      document.getElementById("BANDValue").innerHTML = data[7];
      if (parseInt(data[0]) > 4) {
        document.getElementById("fwd_box").className = "box redbox";
      } else {
        document.getElementById("fwd_box").className = "box";
      }
    }
  };
  xhttp.open("GET", "readDATA", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";