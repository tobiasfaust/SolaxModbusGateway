from delete_old_objects import delete_old_objects
import urllib.parse
import re
import boto3

ressource = boto3.resource('s3')

def lambda_handler(event, context):
    
    releaseJSON = ".+/releases.*\.json"
    
    # Name des Buckets in dem das S3 Put Event aufgetreten ist
    bucketname = event['Records'][0]['s3']['bucket']['name']
    # Name der Datei die das Event ausgelöst hat
    key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')
    
    # Prevent endless loop due writing releases.json
    #if key.endswith(releaseJSON):
    if re.match(releaseJSON, key):
        return "Do nothing because an releases json files was touching"
    
    bucket = ressource.Bucket(bucketname)
    path = key.split("/")
    path.pop()
    targetPath = "/".join(path)
    
    # delete old objects
    delete_old_objects(bucketname, targetPath)

    #exit("MyExit")
    CreateReleasesJson(bucketname, targetPath, "ESP32", 5)
    CreateReleasesJson(bucketname, targetPath, "ESP8266", 5)
    CreateReleasesJson(bucketname, targetPath, "ESP32-S2", 5)
    CreateReleasesJson(bucketname, targetPath, "ESP32-S3", 5)
    CreateReleasesJson(bucketname, targetPath, "ESP32-C3", 5)
    
    
############################################
# Parameters:
#       arch : Architecture, passend der Nomenklatur im Dateinamen -> ESP32|ESP8266
#       history: Anzahl der Releases die im releases.json aufgenommen werden sollen, 0 = nur die aktuelle Version
############################################
def CreateReleasesJson(BucketName, TargetPath, arch, history):   
    s3 = boto3.client('s3')
    
    
    resp = s3.list_objects(Bucket=BucketName,Prefix=TargetPath + "/")
    if 'Contents' in resp:
        objs = resp['Contents']
        
        # schreibe die Releasenummer/Github Action Nummer in ein eigenes Feld für Sortierung 
        for o in resp['Contents']:
            try:
                found = re.search('[\d]+\.[\d]+\-(\d+)\.[DEV|PRE|PROD]', o['Key']).group(1)
            except AttributeError:
                found = '0' # apply your error handling
    
            o['MyNum'] = found

        # sortieren nach Releasenummer
        files = sorted(objs, key=lambda i:i['MyNum'], reverse=True) 
        
        # hilfstabellen
        hashtable = {}
        hashtable = {'DEV':[],'PRE':[],'PROD':[] }
        myJSON = "[ \n"
        
        for key in files:
            for stage in hashtable.keys():
                if re.match(".+\.json", key['Key']) and key['Key'].find("."+arch+".") > 0 and key['Key'].find("."+stage+".") > 0 :
                    if len(hashtable[stage]) <= history : 
                        hashtable[stage].append(key)
                        file_content = ressource.Object(BucketName, key['Key']).get()['Body'].read().decode('utf-8')
                        myJSON += file_content + ","
                        #print ("Save Object #"+str(len(hashtable[stage]))+" for "+arch+"/"+stage+": " + key['Key'])
            
        myJSON = myJSON[:-1]
        myJSON += "]"      

        # Put JSON to S3
        # old releases file was deleted by "delete_old_objects" function
        object = ressource.Object(BucketName, TargetPath + "/releases_" + arch + ".json")
        object.put(Body=myJSON)
    
        # Enable public Access
        object_acl = ressource.ObjectAcl(BucketName, TargetPath + "/releases_" + arch + ".json")
        response = object_acl.put(ACL='public-read')

