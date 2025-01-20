import os, shutil
import json
import logging
 
# Konfigurieren des Loggings 
logging.basicConfig(level=logging.INFO)

def read_json_file(file) -> dict:
    """
    Liest eine JSON-Datei und gibt die Daten zurück.

    <b>Parameter:</b>
        file_path (str): Der Pfad zur JSON-Datei.

    <b>Rückgabewert:</b>
        dict: Die gelesenen JSON-Daten.
    """
    if os.path.isfile(file):
        try:
            with open(file, 'r', encoding='utf-8') as file:
                return json.load(file)
        except json.JSONDecodeError:
            logging.error(f"Warnung: Kann die JSON-Datei nicht lesen: {file}")
        except Exception as e:
            logging.error(f"Fehler beim Verarbeiten von {file}: {e}")
        return None
    else:
        logging.error(f"Warnung: Datei {file} nicht gefunden.")
        return None



def renameDirs(root: str) -> None:
    """
    Iteriert über alle Verzeichnisse im angegebenen Zielverzeichnis 
    und benennt alle Verzeichnisse um, die mit '.zip' enden.
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    for dirpath, dirnames, filenames in os.walk(root):
        for dirname in dirnames:
            # Überprüfen, ob das Verzeichnis mit '.zip' endet
            if dirname.endswith('.zip'):
                # Neuer Name ohne '.zip' Endung
                new_dirname = dirname[:-4]
                # Erstelle den vollständigen Pfad zum alten und neuen Verzeichnis
                old_dirpath = os.path.join(dirpath, dirname)
                new_dirpath = os.path.join(dirpath, new_dirname)
                # Benenne das Verzeichnis um
                os.rename(old_dirpath, new_dirpath)
                logging.info(f"Verzeichnis umbenannt: {old_dirpath} -> {new_dirpath}")


def getAllVariantsFromManifest(root: str) -> list:
    variants = []

    # Durchlaufe alle Unterverzeichnisse und Dateien im Verzeichnis uns finde alle 'manifest.json' Dateien
    # extrahiere daraus den Wert aus dem Key "variant" und speichere diese ohne Dubletten in der Liste
    for dirpath, dirnames, filenames in os.walk(root):
        for filename in filenames:
            if filename == 'manifest.json':
                manifest_path = os.path.join(dirpath, filename)
                try:
                    manifest_data = read_json_file(manifest_path)
                    if manifest_data is not None:
                        variant = manifest_data.get('variant')
                        if variant is not None and variant not in variants:
                            variants.append(variant)
                except json.JSONDecodeError:
                    logging.warning(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
                except Exception as e:
                    logging.error(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")
    
    return variants

def process_filesAll(root: str) -> None:
    filesAll = []

    # Ermittle alle Varianten aus den 'manifest.json' Dateien
    variants = getAllVariantsFromManifest(root)
    for variant in variants:
        filesAll.append(process_manifests(root, variant, 'files.json'))

    path = os.path.join(root, f'filesAll.json')
                                
    # Schreibe die neue JSON-Datei
    save_results_to_json(filesAll, path)

def process_manifestAll(root: str) -> None:
    # Ermittle alle Varianten aus den 'manifest.json' Dateien
    variants = getAllVariantsFromManifest(root)
    
    for variant in variants:
        manifestAll = process_manifests(root, variant, 'manifest.json')
        if manifestAll:
            manifest_path = os.path.join(root, f'manifestAll-{variant}.json')
                                
            # Schreibe die neue JSON-Datei
            save_results_to_json(manifestAll, manifest_path)


def process_manifests(root: str, variant: str, jsonfile: str) -> None:
    """
    Funktion zum Suchen der 'manifest.json', Extrahieren der relevanten Daten und 
    Erstellen der neuen JSON-Datei "manifestAll.json" für jede Variante.
    Erstellt gleichzeitig eine filesAll.json Datei die alle Varianten enthält. Wird für OTA benötigt.
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    
    headerIsWritten = False
    new_manifest_data = {}
    # Durchlaufe alle Unterverzeichnisse und Dateien im Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob jsonfile im aktuellen Verzeichnis existiert
        if jsonfile in filenames:
            manifest_path = os.path.join(dirpath, jsonfile)
            
            try:
                # Öffne und lade die JSON-Daten aus der jsonfile Datei
                manifest_data = read_json_file(manifest_path)
                if manifest_data is not None and manifest_data.get('variant') == variant:
                    if headerIsWritten is False:
                        # Extrahiere die relevanten Informationen: 'name', 'chipFamily', 'version', 'stage' und 'parts'
                        name = manifest_data.get('name')
                        version = manifest_data.get('version')
                        stage = manifest_data.get('stage')
                        build = manifest_data.get('build')
                        #releasetag = manifest_data.get('releasetag', None)
                        
                        # Wenn die erforderlichen Felder vorhanden sind, erstelle die neue 'manifestAll.json' Datei
                        if name and version and stage:
                            headerIsWritten = True
                            # Das neue Dictionary für 'manifestAll.json'
                            new_manifest_data = {
                                "name": name,
                                "version": version,
                                "stage": stage,
                                "build": build, 
                                "variant": variant,
                                #"releasetag": releasetag if releasetag is not None else '',
                                "builds": []  # Wir werden die "builds" später mit chipFamily und parts füllen
                            }
                    # Füge 'chipFamily' und 'parts' als Array zu 'builds' hinzu
                    chipFamily = manifest_data.get('chipFamily')
                    parts = manifest_data.get('parts', [])
                    if chipFamily:
                        new_manifest_data["builds"].append({
                            "chipFamily": chipFamily,
                            "parts": parts
                        })
                            
            except json.JSONDecodeError:
                logging.warning(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
            except Exception as e:
                logging.error(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")
        
    return new_manifest_data
            

def search_manifests_and_extract_version(root: str, keepPath: bool) -> list:
    """
    Funktion zum Suchen und Extrahieren der relevanten Informationen aus manifestAll-*.json-Dateien
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
        keepPath (bool): ob der originale Pfad aus manifestAll-*.json behalten werden soll, ansonsten wird der lokale Pfad gesetzt

    <b>Rückgabewert:</b>
        Liste von JsonObjekten
    """
    results = []  # Liste, um die Ergebnisse zu speichern
    
    # Durchlaufe alle Unterverzeichnisse im angegebenen Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob eine Datei existiert die mit 'manifestAll-*.json' aufhört
        for filename in filenames:
            manifest_path = os.path.join(dirpath, filename)
            if filename.startswith('manifestAll-') and filename.endswith('.json'):
                try:
                    # Öffne und lade die JSON-Daten aus der Datei
                    manifest_data = read_json_file(manifest_path)
                    if manifest_data is not None:
                        
                        # Extrahiere 'version' und 'stage' falls vorhanden
                        version = manifest_data.get('version', None)
                        stage = manifest_data.get('stage', None)
                        build = manifest_data.get('build', None)
                        variant = manifest_data.get('variant', None)
                        #releasetag = manifest_data.get('releasetag', None) 
                        
                        chipFamilies = set()
                        for b in manifest_data.get('builds', []):
                            chipFamily = b.get('chipFamily')
                            if chipFamily:
                                chipFamilies.add(chipFamily)
                        
                        # extrahiere zugehörigen builds aus filesAll.json
                        builds = []
                        files = os.path.join(dirpath, 'filesAll.json')
                        if os.path.isfile(files):
                            with open(files, 'r') as file:
                                items = json.loads(file.read())
                                for item in items:
                                    if item.get('variant') == variant:
                                        builds = item.get('builds', [])
                                        break

                        # Extrahiere den ersten 'path' aus 'parts' falls vorhanden, 
                        # extrahiere daraus den Pfad
                        try:
                            path = manifest_data.get('builds', [])[0].get('parts', [])[0].get('path')
                            #path = os.path.join(os.path.dirname(path), filename)
                        except:
                            path = None
                        
                        if keepPath is False or path is None:
                           # entferne den root-folder "web-installer" aus dem Pfad
                           path =  os.sep.join(manifest_path.strip(os.sep).split(os.sep)[1:])

                        # Falls sowohl 'version', 'build' und 'stage' vorhanden sind, füge sie zum Ergebnis hinzu
                        if version is not None and stage is not None:
                            results.append({
                                'manifest': os.path.join(os.path.dirname(path), filename),
                                'files': os.path.join(os.path.dirname(path), 'filesAll.json'),
                                'version': version,
                                'stage': stage,
                                'variant': variant,
                                'build': int(build) if build else 0,
                                #'releasetag': releasetag if releasetag is not None else '',
                                'chipFamilies': list(chipFamilies),
                                'builds': builds
                            })
                
                except json.JSONDecodeError:
                    logging.warning(f"Warnung: Kann die JSON-Datei nicht lesen: {manifest_path}")
                except Exception as e:
                    logging.error(f"Fehler beim Verarbeiten von {manifest_path}: {e}")
    
    return results


# Funktion zum Speichern der extrahierten Daten in einer neuen JSON-Datei
def save_results_to_json(results, output_file):
    """
    Speichert die extrahierten Daten in einer neuen JSON-Datei.

    <b>Parameter:</b>
        results (dict): Die zu speichernden Ergebnisse.
        output_file (str): Der Pfad zur Ausgabedatei.

    <b>Rückgabewert:</b>
        keiner
    """
    try:
        with open(output_file, 'w', encoding='utf-8') as outfile:
            json.dump(results, outfile, indent=4, ensure_ascii=False)
        logging.info(f"Ergebnisse wurden in {output_file} gespeichert.")
    except Exception as e:
        logging.error(f"Fehler beim Speichern der Ergebnisse: {e}")


def deleteVersions(root: str, keepVersions: int, json: list = None) -> None:
    """
    lädt das json 'versions.json' in 'root' und durchsucht das array darin. Lädt die Attribute 'build' und 'path'. 
    Sortiert alle aufsteigend nach 'build' und entfernt daraus die ersten 'keepVersions' einträge.
    Die 'path' variable zeigt auf die manifestAll datei. Dessen gesamter Ordner wird nun für alle übrigen Einträge gelöscht.

    <b>Parameter:</b>
       root (string): das Root verzeichnis über welches iteriert werden soll
        keepVersions (int): die Anzahl der Build´s, die behalten werden sollen
        json (list): die Liste der Versionen (Optional, falls leer wird es erzeugt)
        
    <b>Rückgabewert:</b>
        keiner
    """
    logging.info(f"Lösche alle Versionen, behalte die letzten {keepVersions} Versionen")
    
    if json is None:
        # Lade die 'versions.json' Datei
        versions_file = os.path.join(root, 'versions.json')
        json = read_json_file(versions_file)
        if json is None:
            logging.error(f"Fehler beim Verarbeiten der Datei {versions_file}")
            return

    # Dictionary zum Speichern der 'build' Nummern und der Pfadangabe
    versions = {}
    
    # Durchlaufe alle Einträge in der 'versions.json' Datei
    for entry in json:
        # Extrahiere 'build' und 'path' falls vorhanden
        build = entry.get('build', None)
        path = entry.get('files', None)
        stage = entry.get('stage', None)
        
        # Wenn 'build' und 'path' vorhanden sind, füge sie zum Dictionary hinzu
        if build is not None and path is not None and stage is not None:
            path = os.path.dirname(path)

            if stage not in versions:
                versions[stage] = {}
            if build not in versions[stage]:
                versions[stage][build] = []
                versions[stage][build].append(path)  # add the path only once
            
    
    # Sortiere die 'build' Nummern aufsteigend pro stage
    for stage in versions:
        sorted_builds = sorted(versions[stage].keys())
        logging.info(f"Folgende Build Nummern wurden für Stage {stage} gefunden: {sorted_builds}")
        # Lösche alle 'build' Nummern, die nicht in den ersten 'keepVersions' enthalten sind
        for build in sorted_builds[:-keepVersions]:
            for path in versions[stage][build]:
                # Lösche den Ordner
                shutil.rmtree(f'web-installer/{path}')
                logging.info(f"web-installer/{path} gelöscht")
    

def changeURL(root: str, url: str) -> None:
    """
    Ändert den URL-Pfad in allen 'manifest.json'-Dateien unterhalb des angegebenen Verzeichnisses 
    in allen 'path' Variablen im Array 'parts' zur angegebenen URL. Speichert die Änderungen in den Dateien.
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
        url (string): die URL, die in den 'path' Variablen geändert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    for dirpath, dirnames, filenames in os.walk(root):
        for filename in filenames:
            if filename in ['manifest.json', 'files.json']:
                manifest_path = os.path.join(dirpath, filename)
                try:
                    manifest_data = read_json_file(manifest_path)
                    if manifest_data is not None:
                        #manifest_data['releasetag'] = TagName # set the release tag
                        parts = manifest_data.get('parts', [])
                        for part in parts:
                            if 'path' in part:
                                old_url = part['path']
                                new_url = os.path.join(url, os.path.basename(old_url))
                                part['path'] = new_url
                        save_results_to_json(manifest_data, manifest_path)
                        logging.info(f"URLs in {manifest_path} geändert.")
                except json.JSONDecodeError:
                    logging.warning(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
                except Exception as e:
                    logging.error(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")

def readOffsetFromPartitionCSV(path: str, name: str) -> int:
    """
    Reads a CSV file and calculates the offset for a given name.
    This function reads the specified CSV file with a delimiter of ','.
    It fills in the Offset column if it is empty by adding the previous offset
    and the value from the Size column of the previous row. It returns the calculated
    offset from the row where the value in the first column matches the given name as a hexadecimal number.
    If the file does not exist or cannot be read, it returns None.
    
    <b>Args:</b>
        path (str): The path to the CSV file.
        name (str): The name to search for in the first column.

    <b>Returns:</b>
        int: The calculated offset as a hexadecimal number, or None if the file cannot be read.
    """

    try:
        with open(path, 'r') as file:
            lines = file.readlines()
            headers = lines[0].strip().split(',')
            name_index = 0
            offset_index = 3
            size_index = 4

            previous_offset = 0
            previous_size = 0
            for line in lines[1:]:
                columns = line.strip().split(',')
                if columns[offset_index].strip() == '':
                    columns[offset_index] = str(previous_offset + previous_size)
                previous_size = int(columns[size_index], 0)
                previous_offset = int(columns[offset_index], 0)
                if columns[name_index] == name:
                    return hex(previous_offset)
    except (FileNotFoundError, IndexError, ValueError):
        return None