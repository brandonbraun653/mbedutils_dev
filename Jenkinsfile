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
                archiveArtifacts artifacts: 'build/host/debug/archive/*',
                                allowEmptyArchive: true,
                                onlyIfSuccessful: true,
                                fingerprint: true
            }
        }
    }
    post {
        always {
            junit('build/host/debug/archive/junit_results.xml')

            publishHTML(
                target: [
                    allowMissing: false,
                    alwaysLinkToLastBuild: false,
                    keepAll: true,
                    reportDir: 'coverage',
                    reportFiles: 'index.html',
                    reportName: 'Code Coverage Report'
                ]
            )

            logParser(
                parsingRulesPath: 'jenkins-log-parser-rules.txt',
                projectRulePath: 'jenkins-log-parser-rules.txt',
                failBuildOnError: true,
                unstableOnWarning: true,
                useProjectRule: true,
                showGraphs: true
            )
        }
    }
}
