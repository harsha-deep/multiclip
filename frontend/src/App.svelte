<script>
  import { onMount } from "svelte";
  import { GetHistory, Copy } from "../wailsjs/go/main/App";

  let items = [];

  async function refresh() {
    items = await GetHistory();
  }

  onMount(refresh);
  setInterval(refresh, 800);
</script>

<main>
  <h2>Multi Clipboard</h2>

  <ul>
    {#each items as item}
      <li>
        <button on:click={() => Copy(item)}>
          {item}
        </button>
      </li>
    {/each}
  </ul>
</main>

<style>
  button {
    width: 100%;
    background: #222;
    color: white;
    border: none;
    padding: 8px;
    text-align: left;
    cursor: pointer;
    border-radius: 4px;
  }

  button:hover {
    background: #333;
  }
</style>