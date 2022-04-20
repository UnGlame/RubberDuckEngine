using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Collections;

namespace Eclipse
{
    public class EclipseBehavior : IScriptable
    {
        private UInt32 gc_handle;
        public GameObject gameObject;
        public Transform transform;

        private List<IEnumerator> coroutineList;

        public UInt32 Entity
        {
            get => gameObject.Entity;
            set => gameObject.Entity = value;
        }

        public string Tag
        {
            get => gameObject.Tag;
        }

        protected void InitBehavior(UInt32 handle, UInt32 entity, string name)
        {
            gameObject = new GameObject(entity, name);
            gc_handle = handle;
            transform = gameObject.transform;
            coroutineList = new List<IEnumerator>();
        }

        public T GetComponent<T>() where T : IScriptable
        {
            if (typeof(T).IsSubclassOf(typeof(EclipseBehavior)))
              return gameObject.GetBehavior<T>() as T;
            else
              return gameObject.GetComponent<T>();
        }

        public T GetComponentInParent<T>() where T : IScriptable
        {
            if (typeof(T).IsSubclassOf(typeof(EclipseBehavior)))
              return gameObject.GetBehaviorInParent<T>() as T;
            else
              return gameObject.GetComponentInParent<T>();
        }

        public void Invoke(string funcName, float time)
        {
            InvokeFunc(Entity, gameObject.ScriptName, funcName, time);
        }

        public void TestTest(UInt32 ent, string text)
        {
            LoopNarrationText(ent, text);
        }

        public float ControllerSensitivity
        {
            get => GetControllerSensitivity();
            set => SetControllerSensitivity(value);
        }

        public float MouseSensitivity
        {
            get => GetMouseSensitivity();
            set => SetMouseSensitivity(value);
        }

        public float Volume
        {
            get => GetVolume();
            set => SetVolume(value);
        }

        public void SetCameraToCinematicRotation(UInt32 entity)
        {
            SetCameraPos(entity);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static GameObject CreateSpotLight(Vector3 pos, Vector3 direction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static GameObject Find(string entName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void InvokeFunc(UInt32 entity, string scriptName, string funcName, float time);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static GameObject CreatePrefab(string prefabName, Vector3 pos, Vector3 rot);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void HighLightObject(UInt32 ent, bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ChangeScene(string sceneName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetSobel(bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetBlur(bool state);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetExposure(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetEmissive(bool value , UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static float GetRandom();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetTextPosition(UInt32 entity1, UInt32 entity2);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetHandObject(UInt32 HandObj, UInt32 HandLightID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetFootObject(UInt32 LeftFootObj, UInt32 RightFootObj, UInt32 FootLight, UInt32 Player);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void UpdateFootPrint();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetTextObject(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetHintTextObject(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetHintTextObject2(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetNarrationText(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void PlayCinematics(string fileName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static bool HasCinematicStopped();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static bool ForceStopCinematics();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void LoopNarrationText(UInt32 entity, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ExitGame();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetBrightness(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static float GetBrightness();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ShowCinematicBlocks(bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float GetControllerSensitivity();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float GetMouseSensitivity();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void SetControllerSensitivity(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void SetMouseSensitivity(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static float GetVolume();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void SetVolume(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void SetCameraPos(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void PlayVideo(string fileName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static bool HasVideoStopped();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SkipVideo();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetPauseFlag(bool State);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetCameraUI(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetGodModeUI(UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ChangeMesh(string sceneName, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void SetTag(UInt32 entity , int number);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void ResetAnimationTime(UInt32 entity);

        private void UpdateCoroutine()
        {
            if (coroutineList.Count <= 0) return;

            for (int i = coroutineList.Count - 1; i != -1; i--)
            {
                IEnumerator enumerator = coroutineList[i];
                WaitForSeconds wait = (WaitForSeconds)enumerator.Current;

                if (wait == null)
                {
                    if (!enumerator.MoveNext())
                        coroutineList.RemoveAt(i);
                    continue;
                }

                wait.duration -= Time.deltaTimeRegardlessPause;

                if (wait.duration < 0.0f)
                {
                    if (!enumerator.MoveNext())
                        coroutineList.RemoveAt(i);
                }
            }
        }

        public void StartCoroutine(IEnumerator enumerator)
        {
            enumerator.MoveNext();
            coroutineList.Add(enumerator);
        }

        public void StopCoroutine(IEnumerator enumerator)
        {
            coroutineList.Remove(enumerator);
        }
    }
}