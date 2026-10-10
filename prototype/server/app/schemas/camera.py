from typing import Optional
from pydantic import BaseModel


class FrameUploadResponse(BaseModel):
    filename: str
    file_path: str
    size_bytes: int
    content_type: str
    uploaded_at: str


class StreamStatus(BaseModel):
    device_id: str
    is_active: bool
    frame_count: int
    last_frame_timestamp: Optional[str] = None
