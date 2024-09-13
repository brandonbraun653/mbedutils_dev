pipeline {
    agent { dockerfile true }
    stages {
        stage('Initialize Repository') {
            steps {
                sh 'cd /workspace/mbedutils_dev'
                sh './update_submodules.sh'
                sh 'cd scripts && ./build_cppumockgen.sh'
            }
        }
    }
}
