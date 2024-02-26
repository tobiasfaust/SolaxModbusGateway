/*******************************
templates anzeigen fuer ItemConfig
https://jsfiddle.net/tobiasfaust/5xh2azd6/

targettable -> die #id der tabelle an das das <template> an tbody angehangen werden soll
template -> die #id des templates
onlyactive -> nur items mit active = 1 werden angezeigt
json -> das json der items
*******************************/
function FillItemConfig(targettable, template, onylactive, json) {
  var tbody = document.querySelector(targettable + " tbody"),
      row = document.querySelector(template),
      tr_tpl,cells,text,openwb_tpl;
  
  json.forEach(function (article) {
    // template für einen Artikel "laden" (lies: klonen)
    tr_tpl = document.importNode(row.content, true);
    
    try {
      openwb_tpl = document.importNode(tr_tpl.querySelector("#openwb").content, true);
    } catch (e) {}
    
    // Zellen befüllen
    cells = tr_tpl.querySelectorAll("td");
    cells.forEach(function (item, index) {
      var text = item.innerHTML;
      text = text.replaceAll("{realname}", article.realname);
      text = text.replaceAll("{name}", article.name);
      text = text.replaceAll("{value}", article.value);
      text = text.replaceAll("{mqtttopic}", article.mqtttopic);
      text = text.replaceAll("{active}", (article.active==1?"checked":""));
            
      item.innerHTML = text;
    });

    if (article.openwbtopic && openwb_tpl) {
      var o = openwb_tpl.querySelector("span");
      o.innerHTML = o.innerHTML.replaceAll("{openwbtopic}", article.openwbtopic);
      cells[2].appendChild(openwb_tpl)
    }    
    
    //template einpassen
    if (article.active == 1 || !onylactive) {
      tbody.appendChild(tr_tpl);
    }
    
  });
}