/** ------------------------------------------------------------
 * SPDX-License-Identifier: GPL-3.0-or-later
 * ------------------------------------------------------------*/

static MirrorSite_t OmarchyCn =
{
  IS_DedicatedMirrorSite,
  "omarchycn", "OmarchyCn", "Omarchy 中国社区自建镜像站", "https://git.zacharyzhang.com/",
  {SKIP, ToFill, ToFill, NULL, ROUGH}
};

static MirrorSite_t Geo =
{
  IS_DedicatedMirrorSite,
  "geo", "Geo", "Arch 官方全球镜像站", "https://geo.mirror.pkgbuild.com/",
  {SKIP, ToFill, ToFill, NULL, ROUGH}
};

#define OS_Pacman_MirrorList "/etc/pacman.d/mirrorlist"
#define OS_Pacman_Omarchy_Conf "/etc/pacman.conf"

#define OS_Omarchy_SelfHosted_Server_Prefix "Server = https://git.zacharyzhang.com"


/** ------------------------------------------------------------
 * omarchy dish (换 Arch 官方源)
 * ------------------------------------------------------------*/

def_dish(os_omarchy, "omarchy");

void
os_omarchy_prepare ()
{
  chef_prep_this_dish (os_omarchy, gs);

  chef_set_recipe_created_on   (this, "2026-09-04");
  chef_set_recipe_last_updated (this, "2026-09-04");

  chef_set_chefs (this, 1, "@yayoinoyume");
  chef_set_sauciers (this, 0);

  chef_set_os_scope (this);
  chef_deny_english(this);
  chef_allow_user_define(this);

  chef_set_note (this,
    "注意: omarchy 的更新脚本 omarchy-refresh-pacman 会覆盖此配置，换源后如被还原，请重新执行 chsrc set omarchy",
    "NOTE: omarchy's updater script omarchy-refresh-pacman overwrites this configuration; if reverted, please rerun chsrc set omarchy");

  def_sources_begin()
  {&UpstreamProvider, "https://stable-mirror.omarchy.org", DelegateToUpstream},
  {&Tuna,             "https://mirrors.tuna.tsinghua.edu.cn/archlinux", DelegateToMirror},
  {&Ustc,             "https://mirrors.ustc.edu.cn/archlinux", DelegateToMirror},
  {&Ali,              "https://mirrors.aliyun.com/archlinux",  DelegateToMirror},
  {&Tencent,          "https://mirrors.tencent.com/archlinux",  DelegateToMirror},
  {&Sjtug_Siyuan,     "https://mirror.sjtu.edu.cn/archlinux", DelegateToMirror},
  {&Bfsu,             "https://mirrors.bfsu.edu.cn/archlinux", DelegateToMirror},
  {&Huawei,           "https://mirrors.huaweicloud.com/archlinux", DelegateToMirror},
  {&Geo,              "https://geo.mirror.pkgbuild.com", DelegateToMirror}
  def_sources_end()
}


void
os_omarchy_getsrc (char *option)
{
  chsrc_view_file (OS_Pacman_MirrorList);
}


/**
 * @consult
 *   1. https://mirrors.tuna.tsinghua.edu.cn/help/archlinux/
 *   2. https://mirrors.tuna.tsinghua.edu.cn/help/archlinuxarm/
 */
void
os_omarchy_setsrc (char *option)
{
  chsrc_ensure_root ();

  chsrc_use_this_source (os_omarchy);

  chsrc_backup (OS_Pacman_MirrorList);

  bool  is_x86 = false;
  bool  is_official = xy_streql (source.url, "https://stable-mirror.omarchy.org");
  char *to_write = NULL;
  char *arch = chsrc_get_cpuarch ();

  if (strncmp(arch, "x86_64", 6)==0)
    {
      is_x86 = true;
      to_write = xy_strcat (3, "Server = ", source.url, "/$repo/os/$arch\n");
      /* 官方源 fallback 放在新源之后兜底(目标即官方时无需重复) */
      if (!is_official)
        to_write = xy_strcat (2, to_write, "Server = https://stable-mirror.omarchy.org/$repo/os/$arch\n");
    }
  else
    {
      is_x86 = false;
      to_write = xy_strcat (3, "Server = ", source.url, "/arm/$arch/$repo\n");
      if (!is_official)
        to_write = xy_strcat (2, to_write, "Server = https://stable-mirror.omarchy.org/arm/$arch/$repo\n");
    }

  /* 幂等: 先清掉 mirrorlist 中所有生效的 Server 行(含之前累积), 再写入新源 + 官方 fallback */
  chsrc_run ("sed -i '/^Server = /d' " OS_Pacman_MirrorList, RunOpt_Default);

  /* 配置文件中，越前面的优先级越高 */
  chsrc_prepend_to_file (to_write, OS_Pacman_MirrorList);

  if (is_x86)
    {
      chsrc_run ("pacman -Syyu", RunOpt_No_Last_New_Line);
    }
  else
    {
      chsrc_run ("pacman -Syy", RunOpt_No_Last_New_Line);
    }

  chsrc_determine_chgtype (ChgType_Auto);
  chsrc_conclude (&source);
}



/** ------------------------------------------------------------
 * omarchyopr dish (换 OPR 专属仓库)
 * ------------------------------------------------------------*/

def_dish(os_omarchyopr, "omarchyopr");

void
os_omarchyopr_prepare ()
{
  chef_prep_this_dish (os_omarchyopr, gs);

  chef_set_recipe_created_on   (this, "2026-09-04");
  chef_set_recipe_last_updated (this, "2026-09-04");

  chef_set_chefs (this, 1, "@yayoinoyume");
  chef_set_sauciers (this, 0);

  chef_set_os_scope (this);
  chef_deny_english(this);
  chef_allow_user_define(this);

  chef_set_note (this,
    "OPR 是 omarchy 官方专属软件包仓库；自建镜像(omarchycn)需导入其 registry key，可额外使用 chsrc set omarchy 来更换 Arch Linux 源",
    "OPR is omarchy's official dedicated package repository; the self-hosted mirror (omarchycn) requires importing its registry key. "
    "You can additionally run chsrc set omarchy to change the Arch Linux source");

  def_sources_begin()
  {&UpstreamProvider, "https://pkgs.omarchy.org/stable/$arch",             DelegateToUpstream},
  {&OmarchyCn,        "https://git.zacharyzhang.com/api/packages/ZacharyZhang-NY/arch/omarchy/x86_64", DelegateToMirror}
  def_sources_end()
}


void
os_omarchyopr_getsrc (char *option)
{
  chsrc_view_file (OS_Pacman_Omarchy_Conf);
}


void
os_omarchyopr_setsrc (char *option)
{
  chsrc_ensure_root ();

  chsrc_use_this_source (os_omarchyopr);

  chsrc_backup (OS_Pacman_Omarchy_Conf);

  /* 检查是否已存在自建镜像行 */
  char *check_cmd = "grep -q '" OS_Omarchy_SelfHosted_Server_Prefix "' " OS_Pacman_Omarchy_Conf;
  int ret = xy_run_get_status (check_cmd);

  /* 幂等保护: 若所选源对应的 Server 行已存在，则无需再改动 pacman.conf */
  char *exists_cmd = xy_strcat (3, "grep -qF 'Server = ", source.url, "' " OS_Pacman_Omarchy_Conf);

  if (xy_streql (source.mirror->code, "upstream"))
    {
      /* 换回官方源: 无条件删除自建镜像行，官方行保持原样 */
      char *sed_cmd = xy_strcat (4, "sed -i '\\|^", OS_Omarchy_SelfHosted_Server_Prefix, ".*|d' ", OS_Pacman_Omarchy_Conf);
      chsrc_run (sed_cmd, RunOpt_Default);
    }
  else if (xy_run_get_status (exists_cmd) == 0)
    {
      chsrc_debug ("recipe", "所选 Server 行已存在，跳过 pacman.conf 修改");
    }
  else if (ret == 0)
    {
      /* 已有自建镜像行，替换为所选源 */
      char *sed_cmd = xy_strcat (6, "sed -i 's|^", OS_Omarchy_SelfHosted_Server_Prefix, ".*|Server = ",
                                  source.url, "|' ", OS_Pacman_Omarchy_Conf);
      chsrc_run (sed_cmd, RunOpt_Default);
    }
  else
    {
      /* 无自建镜像行，在官方行前插入所选源，官方行保留作 fallback */
      char *sed_cmd = xy_strcat (4, "sed -i 's|^Server = https://pkgs.omarchy.org/.*|Server = ",
                                  source.url, "\\n&|' ", OS_Pacman_Omarchy_Conf);
      chsrc_run (sed_cmd, RunOpt_Default);
    }

  /* 自建镜像需导入其 registry key */
  if (xy_streql ("omarchycn", source.mirror->code))
    {
      chsrc_run ("curl -fsSL https://git.zacharyzhang.com/api/packages/ZacharyZhang-NY/arch/repository.key -o /tmp/omarchycn-reg.key", RunOpt_Default);
      chsrc_run ("pacman-key --add /tmp/omarchycn-reg.key", RunOpt_Default);
      chsrc_run ("pacman-key --lsign-key 74DCF57ACD812B24D959F146BD386048867B33B4", RunOpt_Default);
    }

  /* 清理本地陈旧 omarchy.db 缓存: 官方源无 db.sig，omarchycn 镜像有，切换源时残留的旧 sig 会导致 -Syy 验签失败 */
  chsrc_run ("rm -f /var/lib/pacman/sync/omarchy.db /var/lib/pacman/sync/omarchy.db.sig", RunOpt_Default);

  chsrc_run ("pacman -Syy", RunOpt_No_Last_New_Line);

  chsrc_determine_chgtype (ChgType_Auto);
  chsrc_conclude (&source);
}
