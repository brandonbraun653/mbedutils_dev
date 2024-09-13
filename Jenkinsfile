pipeline {
    agent any

    stages {
        stage('Initialize Repository') {
            steps {
                sh 'git submodule update --init --recursive'
            }
        }
    }
}
