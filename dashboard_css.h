const char DB_STYLESHEET[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>

.grid-container {
  display: grid;
  grid-template-columns: 260px 260px 260px;
  grid-template-rows: 50px 30px auto 30px;
  grid-column-gap: 5px;
  grid-row-gap: 5px;
}

.box{
  background: #009879;
  padding: 30px;
  box-sizing: border-box;
  vertical-align: top;
  color: #FFF;
  margin:0px;
  display: inline-block;
}
.innerbox{
  padding: 10px;
  margin-left: auto;
  margin-right: auto;
  display: inline-block;
  text-align: center;
  vertical-align: middle;
}

.fwdtitle {
  grid-column: 1;
  grid-row: 2;
}

.reftitle {
  grid-column: 2;
  grid-row: 2;
}

.vswrtitle {
  grid-column: 3;
  grid-row: 2;
}

.fwdcontent {
  grid-column: 1;
  grid-row: 3;
}

.refcontent {
  grid-column: 2;
  grid-row: 3;
}

.vswrcontent {
  grid-column: 3;
  grid-row: 3;
}

.titlebox  {
  grid-column: 1 / span 2;
  grid-row: 1;
}

.bandbox  {
  grid-column: 3;
  grid-row: 1;
}

.footerbox  {
  grid-column: 1 / span 3;
  grid-row: 4;
  background: #FFFFFF;
  color: #009879;
  border-style: solid;
  border-color: #009879;
  font-family: verdana, sans-serif;
  font-size: 12px;
  color: #009879;
  font-style: normal;
  font-weight: bold;
  font-variant: normal;
  text-align: center;
  letter-spacing: 0px;
  line-height: 20px;
}

.subtitlebox {
  background: #FFFFFF;
  color: #009879;
  border-style: solid;
  border-color: #009879;
  font-family: verdana, sans-serif;
  font-size: 16px;
  color: #009879;
  font-style: normal;
  font-weight: bold;
  font-variant: normal;
  text-align: center;
  letter-spacing: 0px;
  line-height: 20px;
}

.maintitlebox {
  background: #FFFFFF;
  color: #009879;
  border-style: solid;
  border-color: #009879;
  font-family: verdana, sans-serif;
  font-size: 22px;
  color: #009879;
  font-style: normal;
  font-weight: bold;
  font-variant: normal;
  text-align: center;
  letter-spacing: 0px;
  line-height: 50px;
}

.contentbox_text {
  font-family: verdana, sans-serif;
  font-size: 14px;
  color: #FFFFFF;
  font-style: normal;
  font-weight: bold;
  font-variant: normal;
  text-align: center;
  letter-spacing: 0px;
  line-height: 30px;
}

.redbox{
  background: #FFAA79;
}

.button {
  background: none!important;
  border: none;
  padding: 0!important;
  /*optional*/
  font-family: verdana, sans-serif;
  font-size: 12px;
  font-weight: bold;
  /*input has OS specific font-family*/
  color: #009879;
  text-decoration: underline;
  cursor: pointer;
}

</style>
)=====";