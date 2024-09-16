import com.android.build.gradle.AppExtension
import java.io.ByteArrayOutputStream

plugins {
    alias(libs.plugins.agp.app) apply false
}

fun String.execute(currentWorkingDir: File = file("./")): String {
    val byteOut = ByteArrayOutputStream()
    project.exec {
        workingDir = currentWorkingDir
        commandLine = split("\\s".toRegex())
        standardOutput = byteOut
    }
    return String(byteOut.toByteArray()).trim()
}

val gitCommitCount = "git rev-list HEAD --count".execute().toInt()
val gitCommitHash = "git rev-parse --verify --short HEAD".execute()

// also the soname
val moduleId by extra("zn_module_sample")
val moduleName by extra("Zygisk Next Module Sample")
val verName by extra("v1")
val verCode by extra(gitCommitCount)
val commitHash by extra(gitCommitHash)
val abiList by extra(listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64"))

val androidMinSdkVersion by extra(26)
val androidTargetSdkVersion by extra(34)
val androidCompileSdkVersion by extra(34)
val androidBuildToolsVersion by extra("34.0.0")
val androidCompileNdkVersion by extra("27.0.12077973")
val androidSourceCompatibility by extra(JavaVersion.VERSION_17)
val androidTargetCompatibility by extra(JavaVersion.VERSION_17)

tasks.register("Delete", Delete::class) {
    delete(rootProject.buildDir)
}

fun Project.configureBaseExtension() {
    extensions.findByType(AppExtension::class)?.run {
        namespace = "io.github.a13e300.zygisk_next.module.sample"
        compileSdkVersion(androidCompileSdkVersion)
        ndkVersion = androidCompileNdkVersion
        buildToolsVersion = androidBuildToolsVersion

        defaultConfig {
            minSdk = androidMinSdkVersion
        }

        compileOptions {
            sourceCompatibility = androidSourceCompatibility
            targetCompatibility = androidTargetCompatibility
        }
    }

}

subprojects {
    plugins.withId("com.android.application") {
        configureBaseExtension()
    }
    plugins.withType(JavaPlugin::class.java) {
        extensions.configure(JavaPluginExtension::class.java) {
            sourceCompatibility = androidSourceCompatibility
            targetCompatibility = androidTargetCompatibility
        }
    }
}
