/* https://jsfiddle.net/tobiasfaust/p5q9hgsL/ */
/*******************************
reset of rawdata views
*******************************/
function reset_rawdata(rawdatatype) {
  const string_rawdata = document.getElementById(rawdatatype + '_org').innerHTML;
  let bytes = string_rawdata.split(" ");

  bytes = prettyprint_rawdata(rawdatatype, bytes, bytes);
  document.getElementById(rawdatatype).innerHTML = bytes.join(' ');
}

/*******************************
insert Tooltips and linebreaks
*******************************/
function prettyprint_rawdata(rawdatatype, bytearray, bytearray_org) {
  
  for( i=0; i< bytearray.length; i++) {
    const bstr = byte2string(bytearray_org[i]);
    const bint = byte2int(bytearray_org[i]);
    
    bytearray[i] = "<dfn class=\'tooltip_simple\' id=\'" + rawdatatype + "_" + i + "\' onclick=\'cpRawDataPos(" + i + ")\'>" + bytearray[i] + "<span role=\'tooltip_simple\'>Position: " + i + " <hr>Integer: " + bint + "<br>String: " + bstr + "</span></dfn>";
    if (i % 10 == 0) {
      bytearray[i] = ' <br>' + bytearray[i];
    }
  }
  
  return bytearray;
}

/*******************************
take over the clicked byte position into posTextField 
*******************************/
function cpRawDataPos(pos) {
  let posarray;
  const obj = document.getElementById('positions'); 
  
  if(obj.value.trim().length > 0 ) {
    posarray = obj.value.trim().split(" ");
  } else {
    posarray = [];
  }
  
  posarray.push(pos);
  document.getElementById('positions').value = posarray.join(",");
  
}

/*******************************
helper function
*******************************/
function byte2string(bytestring) {
  //return bytestring;
  return String.fromCharCode(parseInt(bytestring, 16));
}

/*******************************
helper function
*******************************/
function byte2int(bytestring) {
  return parseInt(Number(bytestring), 10);
}

/*******************************
compute result from selected positions
*******************************/
function check_rawdata() {
  const datatype = document.querySelector('input[name="datatype"]:checked').value;
  const rawdatatype = document.querySelector('input[name="rawdatatype"]:checked').value;
  const string_positions = document.getElementById('positions').value;
  const string_rawdata = document.getElementById(rawdatatype + '_org').innerHTML;
  
  let   bytes = string_rawdata.split(" ");
  const pos = string_positions.split(",");
  const bytes_org = string_rawdata.split(" ");
  
  // reset all rawdata containers
  reset_rawdata('id_rawdata');
  reset_rawdata('live_rawdata');
  
  let result;
  if (datatype == 'int') { result = 0; }
  if (datatype == 'string') { result = "";}
  
  for( j=0; j< pos.length; j++) {
    if (datatype == 'int') { 
      result = result << 8 | byte2int(bytes[Number(pos[j])]); 
    }
    if (datatype == 'string') { 
      result = result + byte2string(bytes[Number(pos[j])]); 
    }
    
   bytes[Number(pos[j])] = "<span id=\'" + rawdatatype +"_" + Number(pos[j]) + "_val\' style='color: red;'>" + bytes[Number(pos[j])] + "</span>";
  }
  
  document.getElementById('rawdata_result').innerHTML = "= " + result;
  document.getElementById(rawdatatype).innerHTML = prettyprint_rawdata(rawdatatype, bytes, bytes_org).join(' ');
}
