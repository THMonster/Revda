<script setup lang="ts">
import LiveRoomCard from "../components/LiveRoomCard.vue";
import LiveRoomCardHis from "../components/LiveRoomCardHis.vue";
import { Refresh, Setting } from "@element-plus/icons-vue";
import { ElMessage } from "element-plus";
import { ref, onMounted, watch, computed } from "vue";
import { invoke } from "@tauri-apps/api";
import { exit } from "@tauri-apps/api/process";
import useHotkey, { HotKey } from "vue3-hotkey";

interface Room {
  cover: string;
  title: string;
  owner: string;
  is_loading: boolean;
  is_living: boolean;
  is_history: boolean;
  is_saved: boolean;
}

const input = ref("");
const selection = ref("");
const active_tab = ref("first");
const room_info = ref(new Map<string, Room>());
const saved_rooms = ref([""]);
const history_rooms = ref([""]);

const RoomPrefixList = [{
    value: 'do',
    label: '斗鱼直播',
},{
    value: 'hu',
    label: '虎牙直播',
},{
    value: 'bi',
    label: '哔哩'
},{
    value: 'yt',
    label: 'YouTube(Channel)',
},{
    value: 'ytv',
    label: 'YouTube(Video)',
},{
    value: 'tw',
    label: 'Twitch'
}];

let remove_saved_room = (roomCode: string) => {
  let i = saved_rooms.value.indexOf(roomCode);
  if (i !== -1) {
    room_info.value.get(roomCode)!.is_saved = false;
    saved_rooms.value.splice(i, 1);
    invoke("remove_saved_room", { roomCode: roomCode }).catch((e) => {
      console.log(e);
    });
  }
};

let remove_history_room = (roomCode: string) => {
  let i = history_rooms.value.indexOf(roomCode);
  if (i !== -1) {
    room_info.value.get(roomCode)!.is_history = false;
    history_rooms.value.splice(i, 1);
    invoke("remove_history_room", { roomCode: roomCode })
      .then(() => {})
      .catch((e) => {
        console.log(e);
      });
  }
};

let add_new_saved = (roomCode: string) => {
  let i = saved_rooms.value.indexOf(roomCode);
  if (i === -1) {
    room_info.value.get(roomCode)!.is_saved = true;
    if (room_info.value.get(roomCode)!.is_living === true) {
      saved_rooms.value.unshift(roomCode);
    } else {
      saved_rooms.value.push(roomCode);
    }
    invoke("add_saved_room", { roomCode: roomCode })
      .then(() => {
        ElMessage({
          message: "收藏成功！",
          type: "success",
        });
      })
      .catch((e) => {
        console.log(e);
      });
  }
};

let open_room = (roomCode: string) => {
  invoke("open_room", {
    roomCode: roomCode,
  }).then(() => {});
  invoke("get_room_info_by_code", {
    roomCode: roomCode,
  })
    .then((res: any) => {
      if (room_info.value.has(res.room_code)) {
        room_info.value.get(res.room_code)!.is_history = true;
        room_info.value.get(res.room_code)!.cover = res.cover;
        room_info.value.get(res.room_code)!.title = res.title;
        room_info.value.get(res.room_code)!.owner = res.owner;
        room_info.value.get(res.room_code)!.is_living = res.is_living;
        room_info.value.get(res.room_code)!.is_loading = false;
      } else {
        room_info.value.set(res.room_code, {
          cover: res.cover,
          title: res.title,
          owner: res.owner,
          is_loading: false,
          is_living: res.is_living,
          is_history: true,
          is_saved: false,
        });
      }
      let i = history_rooms.value.indexOf(res.room_code);
      if (i !== -1) {
        history_rooms.value.splice(i, 1);
      }
      history_rooms.value.unshift(res.room_code);
      invoke("add_history_room", { roomCode: res.room_code }).catch((e) => {
        console.log(e);
      });
    })
    .catch((e) => {
      console.log(e);
    });
};

let on_search = () => {
    if (!!!selection.value) {
        ElMessage({
            message: '请选择平台',
            type: 'error',
            offset: 80,
        });
        return
    };
    if (!!!input.value) {
        ElMessage({
            message: '请输入房间/视频号',
            type: 'error',
            offset: 80,
        });
        return
    }
    const roomCode = selection.value + '-' + input.value;
    open_room(roomCode);
};

let refresh = async () => {
  room_info.value.clear();
  saved_rooms.value = [];
  history_rooms.value = [];
  try {
    let res: any = await invoke("get_rooms_saved");
    for (let room_code of res) {
      room_info.value.set(room_code, {
        cover: "",
        title: "",
        owner: "",
        is_loading: true,
        is_living: false,
        is_history: false,
        is_saved: true,
      });
      saved_rooms.value.push(room_code);
    }
    let res2: any = await invoke("get_rooms_history");
    for (let room_code of res2) {
      if (room_info.value.has(room_code)) {
        room_info.value.get(room_code)!.is_history = true;
      } else {
        room_info.value.set(room_code, {
          cover: "",
          title: "",
          owner: "",
          is_loading: true,
          is_living: false,
          is_history: true,
          is_saved: false,
        });
      }
      history_rooms.value.unshift(room_code);
    }
  } catch (e) {
    console.log(e);
  }
  for (let k of room_info.value.keys()) {
    invoke("get_room_info_by_code", {
      roomCode: k,
    })
      .then((res: any) => {
        room_info.value.get(k)!.cover = res.cover;
        room_info.value.get(k)!.title = res.title;
        room_info.value.get(k)!.owner = res.owner;
        room_info.value.get(k)!.is_living = res.is_living;
        room_info.value.get(k)!.is_loading = false;
        if (res.is_living && room_info.value.get(k)!.is_saved) {
          saved_rooms.value.sort((first, second) => {
            let fi = room_info.value.get(first)!.is_living;
            let se = room_info.value.get(second)!.is_living;
            if (fi == se) {
              return 0;
            } else if (fi == true) {
              return -1;
            } else {
              return 1;
            }
          });
        }
      })
      .catch((e) => {
        console.log(e);
      });
  }
};

const hotkeys = ref<HotKey[]>([
  {
    keys: ["Alt", "q"],
    preventDefault: true,
    handler(keys) {
      exit();
    },
  },
  {
    keys: ["Alt", "r"],
    preventDefault: true,
    handler(keys) {
      refresh();
    },
  },
]);
const stop = useHotkey(hotkeys.value);

onMounted(() => {
  refresh();
});
</script>

<template>
  <el-row class="el-row">
      <el-col :span="6" :offset="4" class="text-input-col">
      <el-select v-model="selection" class="m-2" placeholder="选择平台" size="large">
        <el-option v-for="item in RoomPrefixList" :key="item.value" :label="item.label" :value="item.value"/>
      </el-select>
    </el-col>
    <el-col :span="6" :offset="0" class="text-input-col">
      <el-input
        class="el-input"
        v-model="input"
        @keyup.enter="open_search"
        size="large"
        placeholder="在此输入房间/视频号"
      ></el-input>
    </el-col>
    <el-col :span="2" :offset="4" class="text-input-col">
      <el-button class="refresh-button" :icon="Refresh" @click="refresh" circle></el-button>
    </el-col>
  </el-row>
  <el-row class="el-row">
    <el-tabs class="el-tabs" v-model="active_tab">
      <el-tab-pane label="收藏" name="first">
        <ul class="card-list">
          <li v-for="(k, i) in saved_rooms" :key="i" class="card-list-item">
            <live-room-card
              :cover="room_info.get(k)?.cover ?? ''"
              :title="room_info.get(k)?.title ?? ''"
              :owner="room_info.get(k)?.owner ?? ''"
              :is_living="room_info.get(k)?.is_living ?? false"
              :loading="room_info.get(k)?.is_loading ?? true"
              :room_code="k"
              @open_room="open_room(k)"
              @remove_saved="remove_saved_room(k)"
            />
          </li>
        </ul>
      </el-tab-pane>
      <el-tab-pane label="历史" name="second">
        <ul class="card-list">
          <li v-for="(k, i) in history_rooms" :key="i" class="card-list-item">
            <live-room-card-his
              :cover="room_info.get(k)?.cover ?? ''"
              :title="room_info.get(k)?.title ?? ''"
              :owner="room_info.get(k)?.owner ?? ''"
              :is_living="room_info.get(k)?.is_living ?? false"
              :loading="room_info.get(k)?.is_loading ?? true"
              :room_code="k"
              @add_saved="add_new_saved(k)"
              @open_room="open_room(k)"
              @remove_room="remove_history_room(k)"
            />
          </li>
        </ul>
      </el-tab-pane>
    </el-tabs>
  </el-row>
</template>

<style scoped>
.el-row {
  margin-left: 0.5rem;
  margin-right: 0.5rem;
  min-height: 3rem;
}
.el-input {
  width: 15rem;
  line-height: 0px;
}
.refresh-button {
  height: 2rem;
  width: 2rem;
  /* margin-left: 0.5rem; */
}
.card-list {
  width: 100%;
  display: flex;
  justify-content: center;
  flex-wrap: wrap;
  padding: 0px;
  list-style-type: none;
  list-style-position: initial;
  list-style-image: initial;
}
.el-tabs {
  width: 100%;
}
</style>
