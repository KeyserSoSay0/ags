using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace AGS.Types
{
    public delegate void PreSaveRoomHandler(ILoadedRoom roomBeingSaved, CompileMessages errors);

	public interface IRoomController
	{
		/// <summary>
		/// Returns the currently loaded room, or null if none is loaded.
		/// RequiredAGSVersion: 3.0.1.35
		/// </summary>
		ILoadedRoom CurrentRoom { get; }
		/// <summary>
		/// Loads the specified room (from the Game.Rooms collection) into memory.
		/// If another room is currently loaded, the user will be prompted to
		/// save it. Returns true if the room was loaded.
		/// RequiredAGSVersion: 3.0.1.35
		/// </summary>
		bool LoadRoom(IRoom roomToLoad);
        /// <summary>
        /// Saves the loaded room to disk. Room data, background images, and mask images.
        /// </summary>
        void Save();
        /// <summary>
        /// Gets the loaded room backround as a <see cref="Bitmap"/> instance.
        /// </summary>
        /// <param name="background">The background index to get.</param>
        /// <returns>A <see cref="Bitmap"/> instance with the selected background. Returns null if background doesn't exist.</returns>
        Bitmap GetBackground(int background);
        /// <summary>
        /// Sets the loaded room's background frame. This change is not permanent until
        /// <see cref="IRoomController.Save"/> is invoked.
        /// </summary>
        /// <param name="background">The background index to set.</param>
        /// <param name="bmp">The image to use for the frame.</param>
        void SetBackground(int background, Bitmap bmp);
        /// <summary>
        /// Deletes the loaded room's background frame. This change is not permanent until
        /// <see cref="IRoomController.Save"/> is invoked.
        /// </summary>
        /// <param name="background">The background index to delete.</param>
        void DeleteBackground(int background);
        /// <summary>
        /// Gets the loaded room specified mask as a <see cref="Bitmap"/> instance.
        /// </summary>
        /// <param name="mask">The mask type to get.</param>
        /// <returns>A <see cref="Bitmap"/> instance with the selected mask type.Return null if none is selected.</returns>
        Bitmap GetMask(RoomAreaMaskType mask);
        /// <summary>
        /// Sets the loaded room specified mask. This change is not permanent until <see cref="IRoomController.Save"/>
        /// is invoked.
        /// </summary>
        /// <param name="mask">The mask type to set.</param>
        /// <param name="bmp">The mask to set.</param>
        void SetMask(RoomAreaMaskType mask, Bitmap bmp);
        /// <summary>
        /// Resets all of the masks in the room back to their default clear value.  This change is not permanent until <see cref="IRoomController.Save"/>
        /// is invoked.
        /// </summary>
        void ResetMasks();
        /// <summary>
        /// Resizes all of the masks in the room.  This change is not permanent until <see cref="IRoomController.Save"/>
        /// is invoked.
        /// </summary>
        /// <param name="doScale">If the mask image should be scaled to the new size or if just the the canvas size should change </param>
        /// <param name="newWidth">The width to resize the masks to.</param>
        /// <param name="newHeight">The height to resize the masks to.</param>
        /// <param name="xOffset">The amount to shift the mask image along the x axis relative to the left side of the mask.</param>
        /// <param name="yOffset">The amount to shift the mask along the y axis relative to the top of the mask.</param>
        void ResizeMasks(bool doScale, int newWidth, int newHeight, int xOffset, int yOffset);
        /// <summary>
        /// Gets the area number on the specified room mask at (x,y)
        /// RequiredAGSVersion: 3.0.1.35
        /// </summary>
        int  GetAreaMaskPixel(RoomAreaMaskType maskType, int x, int y);
        /// <summary>
        /// Draws the room background to the specified graphics context.
        /// RequiredAGSVersion: 3.0.1.35
        /// </summary>
        [Obsolete("The method is deprecated because it takes integer for scale which is inaccurate. Use overload with double for scale instead.")]
        void DrawRoomBackground(Graphics g, int x, int y, int backgroundNumber, int scaleFactor);
		/// <summary>
		/// Draws the room background to the specified graphics context,
		/// and overlays one of the room masks onto it.
		/// RequiredAGSVersion: 3.0.1.35
		/// </summary>
        [Obsolete("The method is deprecated because it takes integer for scale which is inaccurate. Use overload with double for scale instead.")]
		void DrawRoomBackground(Graphics g, int x, int y, int backgroundNumber, int scaleFactor, RoomAreaMaskType maskType, int maskTransparency, int selectedArea);
        /// <summary>
        /// Draws the room background to the specified graphics context,
        /// and overlays one of the room masks onto it.
        /// </summary>
        void DrawRoomBackground(Graphics g, Point point, int backgroundNumber, double scaleFactor, RoomAreaMaskType maskType, int maskTransparency, int selectedArea);
        /// <summary>
        /// Sets the mask resolution and scales all the room's masks according to the
        /// <see cref="Room.MaskResolution"/>, execept for the
        /// <see cref="RoomAreaMaskType.WalkBehinds"/> which retains the
        /// resolution of the background image. If the <see cref="Room.MaskResolution"/>
        /// has a resolution of 2 it will halve the masks in both dimensions.
        /// </summary>
        void AdjustMaskResolution(int maskResolution);
        /// <summary>
        /// Sets whether or not to grey out non-selected masks when drawing the background.
        /// </summary>
        bool GreyOutNonSelectedMasks { set; }
        /// <summary>
        /// Occurs when a room is about to be saved to disk. You can add a new CompileError
        /// to the errors collection if you want to prevent the save going ahead.
        /// RequiredAGSVersion: 3.2.0.95
        /// </summary>
        event PreSaveRoomHandler PreSaveRoom;
	}
}
