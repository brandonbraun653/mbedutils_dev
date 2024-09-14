pipeline {
    agent { dockerfile true }
    stages {
        stage('Initialize Repository') {
            steps {
                sh './update_submodules.sh'
            }
        }
        stage('Run Debug Tests') {
            steps {
                sh '''
                /usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -S$(pwd) -B$(pwd)/build/host/debug -G Ninja
                cmake --build $(pwd)/build/host/debug --parallel $(getconf _NPROCESSORS_ONLN) --target UnitTests
                '''
            }
        }
        stage('Export Artifacts') {
            steps {
                archiveArtifacts artifacts: '$(pwd)/build/host/debug/coverage/*',
                                allowEmptyArchive: true,
                                onlyIfSuccessful: true,
                                fingerprint: true
            }
        }
    }
}
