import {createApp} from "vue";
import App from "./App.vue";
import VueChartkick from "vue-chartkick";
import "chartkick/highcharts"
import "bootstrap/scss/bootstrap.scss"

createApp(App).use(VueChartkick).mount("#app");
