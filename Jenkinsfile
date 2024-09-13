pipeline {
    agent { dockerfile true }
    stages {
        stage('Initialize Repository') {
            steps {
                sh 'cd /workspace/mbedutils_dev && git submodule update --init --recursive || true'
            }
        }
    }
}
