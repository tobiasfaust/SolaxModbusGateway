from delete_old_objects import delete_old_objects
import urllib.parse
import boto3

ressource = boto3.resource('s3')

def lambda_handler(event, context):
    
    #bucketname = "tfa-releases"
    #targetPath = "test/" 
    
    releaseJSON = "releases.json"
    
    bucketname = event['Records'][0]['s3']['bucket']['name']
    key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')
    
    # Prevent endless loop due writing releases.json
    if key.endswith(releaseJSON):
        return "Do nothing if '" + releaseJSON + "' is touching"
    
    bucket = ressource.Bucket(bucketname)
    path = key.split("/")
    path.pop()
    targetPath = "/".join(path)
    
    # delete old objects
    delete_old_objects(bucketname, targetPath)
    
    # Check for old releases.json and delete it
    try:
        bucket.Object(targetPath + "/" + releaseJSON).delete()
    except Exception as e:
        print(e)

    myJSON = "[ \n"
    for obj in bucket.objects.filter(Prefix=targetPath + "/"):
        if obj.key.endswith('.json'):
            file_content = obj.get()['Body'].read().decode('utf-8')
            myJSON += file_content
            
    myJSON += "]"
    
    # Put JSON to S3
    object = ressource.Object('tfa-releases', targetPath + "/" + releaseJSON)
    object.put(Body=myJSON)

    # Enable public Access
    object_acl = ressource.ObjectAcl('tfa-releases', targetPath + "/" + releaseJSON)
    response = object_acl.put(ACL='public-read')
    
