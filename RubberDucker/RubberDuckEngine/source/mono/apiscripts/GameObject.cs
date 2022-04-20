using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;

namespace Eclipse
{
    public class GameObject
    {
        public UInt32 Entity;
        public Transform transform;
        public string ScriptName;

        public string Tag
        {
            get => GetTag(Entity);
        }

        public GameObject(UInt32 entity, string name)
        {
            Entity = entity;
            ScriptName = name;
            transform = new Transform(Entity);
        }

        public T GetComponent<T>() where T : IScriptable
        {
            return (T)Activator.CreateInstance(typeof(T), new object[] { Entity });
        }

        public T GetBehavior<T>() where T : IScriptable
        {
            return getBehavior(Entity, typeof(T).Name) as T;
        }

        public T GetComponentInParent<T>()
        {
            return (T)Activator.CreateInstance(typeof(T), new object[] { getParentEnt(Entity) });
        }

        public T GetBehaviorInParent<T>() where T : IScriptable
        {
            return getBehaviorInParent(Entity, typeof(T).Name) as T;
        }

        public void setText(UInt32 entity, string text)
        {
            SetText(entity, text);
        }

        public bool enabled
        {
            set => setEnabled(Entity, ScriptName,  0);
        }

        public string texture
        {
            set => ChangeTexture(Entity, value);
        }

        public string material
        {
            set => ChangeMaterial(Entity, value);
        }

        public bool visible
        {
            set => setVisibility(Entity, value);
        }

        public void SetActive(bool state)
        {
            setActive(Entity, state ? 1 : 0);
        }

        public void Destroy()
        {
            Destroy(Entity, 0.0f);
        }

        public void DestroyEntity(UInt32 entity, float timer)
        {
            Destroy(entity, timer);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void setActive(UInt32 entity, int state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void setEnabled(UInt32 entity, string name, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void ChangeTexture(UInt32 ent, string texName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void ChangeMaterial(UInt32 ent, string texName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static GameObject getBehavior(UInt32 entity, string behaviorName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static GameObject getBehaviorInParent(UInt32 entity, string behaviorName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static UInt32 getParentEnt(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void SetText(UInt32 entity, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string GetTag(UInt32 ent);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string Destroy(UInt32 ent, float timer);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string setVisibility(UInt32 ent, bool value);
    }
}