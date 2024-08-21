import subprocess; 
import sys;
import os;

def git_branch():
    print('-D GIT_BRANCH=\\"%s\\"' % subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip().decode())

def git_repo():
    output = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'])
    repo = os.path.basename(output).strip().decode()
    print('-D GIT_REPO=\\"%s\\"' % repo);



if __name__ == '__main__':
    globals()[sys.argv[1]]()

    
