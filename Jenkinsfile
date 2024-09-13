pipeline {
    agent any

    stages {
        stage('Initialize Repository') {
            steps {
                ./update_submodules.sh
            }
        }
    }
}
