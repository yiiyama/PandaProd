//// DEFINE THESE PLEASE ////

def in_files = ['16F85498-2E38-E711-BDFE-5065F3815281', 'F6F880D2-ACBE-E611-B59D-0CC47A706D40']

def cmssw_version = 'CMSSW_8_0_29'
def panda_tree_user = 'dabercro'
def panda_tree_branch = 'jenkins-clean'

//// Hopefully you don't need to change much of what follows

def do_src = 'export SCRAM_ARCH=slc6_amd64_gcc530; source /cvmfs/cms.cern.ch/cmsset_default.sh'

def prComment(message) {
  withCredentials([usernameColonPassword(credentialsId: 'mitsidekick', variable: 'USERPASS')]) {
    sh '''
       set +x
       if [ ! -z $CHANGE_URL ]
       then
           # Craft the URL for the github comments API from the PR
           COMMENT_URL=$(echo $CHANGE_URL | perl -a -F/ -ne 'chomp @F[-1]; print "https://api.github.com/repos/@F[-4]/@F[-3]/issues/@F[-1]/comments"')
           # Lots of escape strings because Jenkins uses this sh string to generate a .sh file...
           curl -u $USERPASS -X POST --data-binary \"{\\\"body\\\": \\\"''' + message + '''\\\"}\" $COMMENT_URL
       fi
       '''
  }
}

def produce_panda(cmssw_version, do_src, base) {
  def in_files_dir = '/mnt/hadoop/scratch/jenkins/miniaod'
  return {
    try {
      dir ("${cmssw_version}/src/PandaProd/Producer/cfg") {
        sh do_src + '''
           eval `scramv1 runtime -sh`
           BASE=''' + base + '''
           cmsRun $(perl -ne '/(data|mc)/ && print $1' $HOME/miniaod/$BASE.txt).py inputFiles=file:''' + in_files_dir + '''/$BASE.root outputFile=$BASE.root maxEvents=1000
           '''
      }
    }
    catch (all) {
      prComment('Problem producing with ``$(head -n1 ${HOME}/miniaod/' + base + '.txt)``. See ${BUILD_URL}console')
      error("Failed to produce ${base}.root")
    }
  }
}

def run_test(cmssw_version, do_src, base) {
  return {
    try {
      dir ("${cmssw_version}/src") {
        sh do_src + '''
           eval `scramv1 runtime -sh`
           BASE=''' + base + '''
           testpanda PandaProd/Producer/cfg/$BASE.root ${BUILD_TAG}_$(tail -n1 $HOME/miniaod/$BASE.txt)
           '''
      }
    }
    catch (all) {
      prComment('Problem testing ``' + base + '.root``. See ${BUILD_URL}console')
      error("Failed on test of ${base.root}")
    }
  }
}

node {
  checkout scm

  stage('Getting Source') {
    try {
      prComment('Starting tests! You can follow along (if on T3).\\n\\n- Job: ${JOB_URL}\\n- Run steps: ${BUILD_URL}flowGraphTable')

      sh 'ls'
      // Get CMSSW if needed
      sh """
         if [ ! -d ${cmssw_version} ]
         then
             ${do_src}
             scramv1 project CMSSW ${cmssw_version}
         fi
         """
      dir ("${cmssw_version}/src") {
        sh 'ls'
        // Make PandaTree
        sh '''
           if [ -d PandaTree ]
           then
               rm -rf PandaTree
           fi
           ''' + "git clone -b ${panda_tree_branch} https://www.github.com/${panda_tree_user}/PandaTree.git"

        // Make PandaProd directory for us to recursively copy files into
        sh '''
           if [ -d PandaProd ]
           then
               rm -rf PandaProd
           fi
           mkdir PandaProd
           '''
      }
  
      // Copy all of the files, and the HEAD where we will get the git tag from
      sh "cp --parents `git ls-files` ${cmssw_version}/src/PandaProd"
      sh "cp --parents .git/HEAD ${cmssw_version}/src/PandaProd"

      // Install the other CMSSW packages, if needed
      dir ("${cmssw_version}/src") {
        sh do_src + '''
           eval `scramv1 runtime -sh`

           PandaProd/Producer/scripts/create-manifest test.txt
           # Make sure the file is made, even for new repos
           touch test.txt

           # Don't code drunk on Thanksgiving, kids
           # Basically supposed to install only individual packages that don't match in case there's an update
           for PKG in $(diff test.txt PandaProd/Producer/scripts/manifest.txt | perl -ne '/ (\\w+\\/\\w+)\\// && print "$1\\n"' | sort | uniq)
           do
               PandaProd/Producer/scripts/install-pkg $(PKG=$PKG perl -ane '/INSTALL.*$ENV{PKG}/ && print "@F[1] @F[2]\n"' PandaProd/Producer/cfg/setuprel.sh) $PKG
           done

           # Do patching
           eval $(grep sed PandaProd/Producer/cfg/setuprel.sh)
           '''
      }
    }
    catch (all) {
      prComment('Problem gathering source. See ${BUILD_URL}console')
      error('Failed gathering source files')
    }
  }

  stage('Compiling') {
    try {
      dir ("${cmssw_version}/src") {
        // Compile
        sh "${do_src}; eval `scramv1 runtime -sh`; scram b -j4"
      }
    }
    catch (all) {
      prComment('Problem compiling. See ${BUILD_URL}console')
      error('Failed to compile')
    }
  }

  stage('Producing Pandas') {
    def begin_line = '\\n- ``$(head -n1 ${HOME}/miniaod/'
    def end_line = '.txt)``'
    def file_string = in_files.join("${end_line}${begin_line}")
    def result = 'Finished compiling! Will try to run over:\\n' + begin_line + file_string + end_line
    prComment(result)

    parallel in_files.collectEntries {
      ["Produce ${it}" : produce_panda(cmssw_version, do_src, it)]
    }
  }

  stage('Checking Pandas') {
    parallel in_files.collectEntries {
      ["Test ${it}" : run_test(cmssw_version, do_src, it)]
    }
  }

  stage('Finishing') {

    try {
      // Copy root files to hadoop for other repository's tests
      dir ("${cmssw_version}/src/PandaProd/Producer/cfg") {
        sh '''
           # Copy over individual branch result
           if [ -z $CHANGE_URL ]
           then
               mkdir -p /mnt/hadoop/scratch/jenkins/panda/${JOB_NAME}
               cp *.root /mnt/hadoop/scratch/jenkins/panda/${JOB_NAME}
           fi
           '''
      }
    }
    catch (all) {
      prComment ('Problem finishing up. See ${BUILD_URL}console')
      error ('Final steps failed')
    }

    def begin_line = '\\n- http://t3serv001.mit.edu/~${USER}/relval/?d=${BUILD_TAG}_$(tail -n1 ${HOME}/miniaod/'
    def end_line = '.txt)'
    def file_string = in_files.join("${end_line}${begin_line}")
    def result = 'Tests passed! See plots at:\\n' + begin_line + file_string + end_line

    prComment(result)
  }
}
