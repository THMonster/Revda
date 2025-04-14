<script setup lang="ts">
import { ref } from "vue";
import { Picture as IconPicture, StarFilled } from "@element-plus/icons-vue";
import { ElMessage } from "element-plus";
import { invoke } from "@tauri-apps/api";

const props = defineProps<{
  cover: string;
  title: string;
  owner: string;
  room_code: string;
  is_living: boolean;
  loading: boolean;
}>();

const emit = defineEmits(["remove_saved", "open_room"]);
</script>

<template>
  <div class="room-card">
    <div class="room-card-cover" @click.stop="emit('open_room')">
      <el-image fit="cover" :src="props.cover">
        <template #error>
          <div class="el-image-fallback">
            <el-icon>
              <icon-picture />
            </el-icon>
          </div>
        </template>
      </el-image>
      <el-tag
        v-if="props.is_living"
        class="living-tag"
        type="success"
        size="small"
        effect="plain"
        color="#ffffffbb"
      >直播中</el-tag>
      <el-row class="icon-row" justify="end">
        <el-icon class="room-card-icon" size="2rem" color="#F56C6C">
          <star-filled class="room-card-icon-inner" @click.stop="emit('remove_saved')" />
        </el-icon>
      </el-row>
    </div>
    <el-skeleton :loading="props.loading" animated>
      <template #template>
        <el-row class="room-card-text-row" justify="start">
          <el-skeleton-item class="room-card-title" variant="p" />
        </el-row>
        <el-row class="room-card-text-row" justify="start">
          <el-skeleton-item class="room-card-owner" variant="p" style="width: 50%" />
        </el-row>
      </template>
      <template #default>
        <el-row class="room-card-text-row" justify="start">
          <p class="room-card-title">{{ props.title }}</p>
        </el-row>
        <el-row class="room-card-text-row" justify="start">
          <p class="room-card-owner">{{ props.owner }}</p>
        </el-row>
      </template>
    </el-skeleton>
  </div>
</template>

<style scoped>
.room-card-cover {
  position: relative;
}
.room-card-cover:hover .icon-row {
  opacity: 1;
}
.room-card {
  margin: 0.5rem;
}
.el-image {
  border-radius: 8px;
  width: 16rem;
  height: 10rem;
}
.el-image-fallback {
  display: flex;
  justify-content: center;
  align-items: center;
  width: 100%;
  height: 100%;
  background: #f5f7fa;
}
.living-tag {
  top: 0.3rem;
  left: 0.3rem;
  position: absolute;
}
.icon-row {
  width: 100%;
  bottom: 0.6rem;
  position: absolute;
  opacity: 0;
  transition: all 0.3s;
}
.room-card-title {
  margin: 0;
  margin-bottom: 0.2rem;
  font-size: 1rem;
  max-width: 16rem;
  white-space: nowrap;
  text-overflow: ellipsis;
  overflow: hidden;
  color: light-dark(#2c3e50, #e5e5e5);
}
.room-card-owner {
  margin: 0;
  margin-bottom: 0.2rem;
  font-size: 0.7rem;
  max-width: 16rem;
  white-space: nowrap;
  text-overflow: ellipsis;
  overflow: hidden;
  color: light-dark(#2c3e50, #e5e5e5);
}
</style>
