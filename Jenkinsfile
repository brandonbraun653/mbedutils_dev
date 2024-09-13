pipeline {
    agent any

    stages {
        stage('Initialize Repository') {
            steps {
                shell('git submodule update --init --recursive')
            }
        }
    }
}
