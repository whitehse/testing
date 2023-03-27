<main>
  <!--
  Hey!
  <div class="bg-green-300 border-green-600 border-b p-4 m-4 rounded">
    Hello World
  </div>
  -->

  <div>
    <Router {routes}/>
  </div>
</main>

<style global lang="postcss">
  @tailwind utilities;
  @tailwind components;
  @tailwind base;
</style>

<script>
  import Router from 'svelte-spa-router'
  import routes from './routes'
</script>
