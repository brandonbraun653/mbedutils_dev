pipeline {
    agent { dockerfile true }
    stages {
        stage('Initialize Repository') {
            steps {
                sh 'echo $pwd'
                sh './update_submodules.sh'
                sh 'cd scripts && ./build_cppumockgen.sh'
            }
        }
    }
}
