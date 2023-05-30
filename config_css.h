const char CFG_STYLESHEET[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>

.styled-table{
  border-collapse: collapse;
  margin: 25px 0;
  font-size: 0.9em;
  font-family: sans-serif;
  min-width: 400px;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
}

.styled-table thead tr{
  background-color: #009879;
  color: #ffffff;
  text-align: left;
}

.styled-table tbody tr{
  border-bottom: 1px solid #dddddd;
}

.styled-table tbody tr:nth-of-type(even){
  background-color: #f3f3f3;
}
.styled-table tbody tr:last-of-type{
  border-bottom: 2px solid #009879;
}

.styled-table tbody tr.active-row{
  font-weight: bold;
  color: #009879;
}

.backend_button{
  background-color: #009879;
  border: none;
  color: white;
  padding: 5px 5px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  margin: 4px 2px;
  cursor: pointer;
  border-radius: 8px;
}

</style>
)=====";