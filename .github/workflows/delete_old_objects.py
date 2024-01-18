import boto3
import re

s3 = boto3.client('s3')

#get_last_modified = lambda obj: int(obj['LastModified'].strftime('%Y%m%d%H%M%S'))
save_versions = 5

def delete_old_objects(bucketname, targetpath):
    resp = s3.list_objects(Bucket=bucketname,Prefix=targetpath + "/")
    if 'Contents' in resp:
        objs = resp['Contents']
        
        # schreibe die Releasenummer/Github Action Nummer in ein eigenes Feld für Sortierung 
        for o in resp['Contents']:
            try:
                found = re.search('[\d]+\.[\d]+\-(\d+)\.[DEV|PRE|PROD]', o['Key']).group(1)
            except AttributeError:
                found = '0' # apply your error handling
    
            o['MyNum'] = found

        # sortieren nach Dateidatum, fehlerhaft bei wiederherstellung aus Backup!
        #files = sorted(objs, key=get_last_modified, reverse=True) 
        
        # sortieren nach Releasenummer
        files = sorted(objs, key=lambda i:i['MyNum'], reverse=True) 
        
        # hilfstabellen
        hashtable = {}
        hashtable = {'ESP8266': {'DEV':[],'PRE':[],'PROD':[]}, 
                     'ESP32': {'DEV':[],'PRE':[],'PROD':[]},
                     'ESP32-S2': {'DEV':[],'PRE':[],'PROD':[]},
                     'ESP32-S3': {'DEV':[],'PRE':[],'PROD':[]},
                     'ESP32-C3': {'DEV':[],'PRE':[],'PROD':[]}
                    }
        
        for key in files:
            key['save']=0
            
            for arch in hashtable.keys():
                for stage in hashtable[arch].keys():
                    if key['Key'].find("."+arch+".") > 0 and key['Key'].find("."+stage+".") > 0 :
                        if len(hashtable[arch][stage]) <= ((save_versions * 2) - 1) : # 4 Binaries + 4 Json (incl. dem jetzt kommenden)
                            key['save']=1
                            hashtable[arch][stage].append(key)
                            #print ("Save Object #"+str(len(hashtable[arch][stage]))+" for "+arch+"/"+stage+": " + key['Key'])
            
            if key['save']==0:
                #print("Delete this Object: " + key['Key'])
                s3.delete_object(Bucket=bucketname, Key=key['Key'])   
 
