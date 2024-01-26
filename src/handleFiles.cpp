#include "handleFiles.h"

handleFiles::handleFiles() {

}

void handleFiles::GetWebContentConfig(AsyncResponseStream *response) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  response->print("<form id='DataForm'>\n");
  response->print("<table id='maintable' class='editorDemoTable'>\n");
  response->print("<thead>\n");
  response->print("<tr>\n");
  response->print("<td style='width: 250px;'>Name</td>\n");
  response->print("<td style='width: 200px;'>Wert</td>\n");
  response->print("</tr>\n");
  response->print("</thead>\n");
  response->print("<tbody>\n");



  response->print("</tbody>\n");
  response->print("</table>\n");

  response->print("</form>\n\n<br />\n");
  response->print("<form id='jsonform' action='xxxxxxx' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  response->print("  <input type='text' id='json' name='json' />\n");
  response->print("  <input type='submit' value='Speichern' />\n");
  response->print("</form>\n\n");
}