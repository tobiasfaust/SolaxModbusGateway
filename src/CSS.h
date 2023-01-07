#ifndef CSS_H
#define CSS_H

const char STYLE_CSS[] PROGMEM = R"=====(
 /* https://www.peterkropff.de/site/css/kontext_selektoren.htm */
 
 body {
   font-size: 140%;
   font-family: Verdana,Arial,Helvetica,sans-serif;
 }
 
 input[type="number"] {
  width: 3em;
 }
 
 .inline {
   float: left;
   clear: both;
 }
 
 .hide {
   display: none;
 }
 
 input[type="submit"] {
     padding: 4px 16px;
     margin: 4px;
     background-color: #07D;
     color: #FFF;
     text-decoration: none;
     border-radius: 4px;
     border: none;
 }
 input[type="submit"]:hover { background: #369; }
 
 input, select, textarea {
     margin: 4px;
     padding: 4px 8px;
     border-radius: 4px;
     background-color: #eee;
     border-style: solid;
     border-width: 1px;
     border-color: gray;
 }
 input:hover, select:hover, textarea:hover { background-color: #ccc; }
 
 .editorDemoTable {
     border-spacing: 0;
     background-color: #FFF8C9;
     margin-left: auto;
     margin-right: auto;
 }
 .editorDemoTable thead {
     color: #000000;
     background-color: #2E6C80;
     text-align: center;
 }
 .editorDemoTable thead td {
     font-weight: bold;
     font-size: 13px;
 }
 .editorDemoTable td {
     border: 1px solid #777;
     margin: 0 !important;
     padding: 2px 3px;
     font-size: 11px;
 } 
 .errortext {
   color: red;
   font-size: 13px;
   text-align: center;
 }
  .navi {
   border-bottom: 3px solid #777;
   padding: 5px;
   text-align: center;
   font-size: 13px;
 }
 .navi_active {
   background-color: #CCCCCC;
   border: 3px solid #777;;
   border-bottom: none;
 }

 .ajaxchange {
   color: red;
 }
 
.ButtonRefresh {
   font-size: 13px;
   background-color: #EEEEEE; 
   color: #999999;
}


/* https://proto.io/freebies/onoff/ */
    .onoffswitch {
        position: relative; width: 46px;
        -webkit-user-select:none; -moz-user-select:none; -ms-user-select: none;
    }
    .onoffswitch-checkbox {
        display: none;
    }
    .onoffswitch-label {
        display: block; overflow: hidden; cursor: pointer;
        border: 2px solid #999999; border-radius: 20px;
    }
    .onoffswitch-inner {
        display: block; width: 200%; margin-left: -100%;
        transition: margin 0.3s ease-in 0s;
    }
    .onoffswitch-inner:before, .onoffswitch-inner:after {
        display: block; float: left; width: 50%; height: 15px; padding: 0; line-height: 15px;
        font-size: 10px; color: white; font-family: Trebuchet, Arial, sans-serif; font-weight: bold;
        box-sizing: border-box;
    }
    .onoffswitch-inner:before {
        content: "ON";
        padding-left: 5px;
        background-color: #34A7C1; color: #FFFFFF;
    }
    .onoffswitch-inner:after {
        content: "OFF";
        padding-right: 5px;
        background-color: #EEEEEE; color: #999999;
        text-align: right;
    }
    .onoffswitch-switch {
        display: block; width: 8px; margin: 3.5px;
        background: #FFFFFF;
        position: absolute; top: 0; bottom: 0;
        right: 27px;
        border: 2px solid #999999; border-radius: 20px;
        transition: all 0.3s ease-in 0s; 
    }
    .onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-inner {
        margin-left: 0;
    }
    .onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-switch {
        right: 0px; 
    }

/* https://wiki.selfhtml.org/wiki/CSS/Tutorials/Tooltips_mit_CSS */
/* used at modbusitem and rawdata page*/

.tooltip {
	font-style: normal;
  cursor: help;
	position: relative;
}

.tooltip span[role=tooltip] {
	display: none;
}

.tooltip:hover span[role=tooltip] {
	display: block;
	position: absolute;
	bottom: 1em;
	left: -6em;
	padding: 0.5em;
	z-index: 100;
	color: #000;
	background-color: #ffebe6;
	border: solid 1px #c32e04;
	border-radius: 0.2em;
  font-style: normal;
}


)=====";

#endif
