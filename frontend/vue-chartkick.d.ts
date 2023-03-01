declare module 'vue-chartkick' {
    import PluginInstallFunction from "vue";

    declare const VueChartkick: {
        install: PluginInstallFunction;
    };
    export default VueChartkick;
}