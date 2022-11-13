var ss = SpreadsheetApp.openById('*******');
var sheet = ss.getSheetByName('*****');

function doGet(e) {
  Logger.log(JSON.stringify(e));
  //----------------------------------------------------------------------------------
  //write_google_sheet() function in esp32 sketch, is send data to this code block
  //----------------------------------------------------------------------------------
  //get gps data from ESP32
  if (e.parameter == undefined) {
    return ContentService.createTextOutput("undefined1");
  }else if(e.parameter.read == "read"){
    return ContentService.createTextOutput(sheet.getRange('A2').getValue());
  }else if(e.parameter.write == undefined){
    return ContentService.createTextOutput("undefined2");
  }else if(e.parameter.write !== ""){
    sheet.getRange("B2").setValue(e.parameter.write);
      return ContentService.createTextOutput("B2 updated (" + e.parameter.write + ").");
  }
  
  return ContentService.createTextOutput("empty");
  //----------------------------------------------------------------------------------
}

//Extra Function. Not used in this project.
//planning to use in future projects.
//this function is used to handle POST request
function doPost(e) {
  var val = e.parameter.value;

  if (e.parameter.value !== undefined) {
    var range = sheet.getRange('A2');
    range.setValue(val);
  }
}