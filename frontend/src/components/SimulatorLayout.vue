<template>
  <div class="container-fluid">
    <div class="row">
      <div class="col-9">
        <slot name="main"></slot>
      </div>
      <div class="col-3 pe-0">
        <slot name="sidebar"></slot>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import {defineComponent} from "vue";

export default defineComponent({
  name: "SimulatorLayout"
});
</script>

<style lang="scss" scoped>

</style>