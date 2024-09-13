pipeline {
    agent any

    stages {
        stage('Initialize Repository') {
            steps {
                sh './update_submodules.sh'
            }
        }
    }
}
